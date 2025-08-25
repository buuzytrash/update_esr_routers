#include "ssh.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <libssh2.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "logging.h"

static LIBSSH2_SESSION *open_session(int *out_sock, const char *ip,
                                     const Config *c) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    inet_aton(ip, &sin.sin_addr);

    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
        log_msg(LOG_ERROR, "SSH connect %s: %s", ip, strerror(errno));
        close(sock);
        return NULL;
    }
    if (libssh2_init(0) != 0) {
        log_msg(LOG_ERROR, "libssh2 init failed");
        close(sock);
        return NULL;
    }

    LIBSSH2_SESSION *sess = libssh2_session_init();
    if (!sess) {
        libssh2_exit();
        close(sock);
        return NULL;
    }
    libssh2_session_set_blocking(sess, 1);

    if (libssh2_session_handshake(sess, sock)) {
        char *err;
        int len;
        libssh2_session_last_error(sess, &err, &len, 0);
        log_msg(LOG_ERROR, "Handshake failed: %.*s", len, err);
        libssh2_session_free(sess);
        libssh2_exit();
        close(sock);
        return NULL;
    }

    int rc;
    if (c->ssh_key_path) {
        rc = libssh2_userauth_publickey_fromfile(sess, c->user, NULL,
                                                 c->ssh_key_path, c->pass);
    } else {
        rc = libssh2_userauth_password(sess, c->user, c->pass);
    }
    if (rc) {
        log_msg(LOG_ERROR, "SSH auth failed");
        libssh2_session_disconnect(sess, "Auth failure");
        libssh2_session_free(sess);
        libssh2_exit();
        close(sock);
        return NULL;
    }

    *out_sock = sock;
    return sess;
}

int ssh_execute(const char *ip, const Config *c, const char *cmd) {
    int sock;
    LIBSSH2_SESSION *sess = open_session(&sock, ip, c);
    if (!sess) return -1;

    LIBSSH2_CHANNEL *ch = libssh2_channel_open_session(sess);
    libssh2_channel_exec(ch, cmd);

    char tmp[256];
    while (libssh2_channel_read(ch, tmp, sizeof(tmp)) > 0);

    libssh2_channel_wait_eof(ch);
    libssh2_channel_close(ch);
    libssh2_channel_wait_closed(ch);

    libssh2_channel_free(ch);
    libssh2_session_disconnect(sess, "Done");
    libssh2_session_free(sess);
    libssh2_exit();
    close(sock);
    return 0;
}

int ssh_execute_capture(const char *ip, const Config *c, const char *cmd,
                        char *out, size_t outlen) {
    int sock;
    LIBSSH2_SESSION *sess = open_session(&sock, ip, c);
    if (!sess) return -1;

    LIBSSH2_CHANNEL *ch = libssh2_channel_open_session(sess);
    libssh2_channel_exec(ch, cmd);

    size_t pos = 0;
    while (pos + 1 < outlen) {
        ssize_t n = libssh2_channel_read(ch, out + pos, outlen - 1 - pos);
        if (n <= 0) break;
        pos += n;
    }
    out[pos] = '\0';

    libssh2_channel_wait_eof(ch);
    libssh2_channel_close(ch);
    libssh2_channel_wait_closed(ch);

    libssh2_channel_free(ch);
    libssh2_session_disconnect(sess, "Done");
    libssh2_session_free(sess);
    libssh2_exit();
    close(sock);
    return 0;
}

int ssh_execute_stream(const char *ip, const Config *c, const char *cmd,
                       void (*on_output)(const char *),
                       void (*on_progress)(int)) {
    int sock;
    LIBSSH2_SESSION *sess = open_session(&sock, ip, c);
    if (!sess) return -1;

    LIBSSH2_CHANNEL *ch = libssh2_channel_open_session(sess);
    libssh2_channel_exec(ch, cmd);

    char raw[256], clean[256];
    ssize_t n;
    while ((n = libssh2_channel_read(ch, raw, sizeof(raw) - 1)) > 0) {
        raw[n] = '\0';
        // Strip ANSI escapes and '\r'
        char *r = raw, *w = clean;
        while (*r) {
            if (*r == 0x1B && *(r + 1) == '[') {
                r += 2;
                while (*r && !isalpha((unsigned char)*r)) ++r;
                if (*r) ++r;
            } else if (*r != '\r') {
                *w++ = *r++;
            } else {
                ++r;
            }
        }
        *w = '\0';

        on_output(clean);
        if (on_progress) {
            int pct;
            if (sscanf(clean, "%*[^0-9]%d%%", &pct) == 1) on_progress(pct);
        }
    }

    libssh2_channel_wait_eof(ch);
    libssh2_channel_close(ch);
    libssh2_channel_wait_closed(ch);

    libssh2_channel_free(ch);
    libssh2_session_disconnect(sess, "Done");
    libssh2_session_free(sess);
    libssh2_exit();
    close(sock);
    return 0;
}
