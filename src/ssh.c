#include "ssh.h"

#include <arpa/inet.h>
#include <errno.h>
#include <libssh2.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "logging.h"

static LIBSSH2_SESSION *open_session(int *out_sock, const char *ip,
                                     const Config *c) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin = {AF_INET, htons(22)};
    inet_aton(ip, &sin.sin_addr);
    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
        log_msg(LOG_ERROR, "SSH connect %s: %s", ip, strerror(errno));
        return NULL;
    }
    libssh2_init(0);
    LIBSSH2_SESSION *sess = libssh2_session_init();
    libssh2_session_set_blocking(sess, 1);
    int rc = libssh2_session_handshake(sess, sock);
    if (rc) {
        char eb[256];
        int el = libssh2_session_last_error(sess, eb, sizeof(eb), NULL);
        log_msg(LOG_ERROR, "Handshake failed: %.*s", el, eb);
        libssh2_session_free(sess);
        close(sock);
        libssh2_exit();
        return NULL;
    }
    if (c->ssh_key_path)
        rc = libssh2_userauth_publickey_fromfile(sess, c->user, NULL,
                                                 c->ssh_key_path, c->pass);
    else
        rc = libssh2_userauth_password(sess, c->user, c->pass);
    if (rc) {
        log_msg(LOG_ERROR, "Auth failed");
        libssh2_session_disconnect(sess, "");
        libssh2_session_free(sess);
        close(sock);
        libssh2_exit();
        return NULL;
    }
    *out_sock = sock;
    return sess;
}

int ssh_execute(const char *ip, const Config *c, const char *cmd) {
    int sock;
    LIBSSH2_SESSION *sess = open_session(&sock, ip, c);
    if (!sess) return -1;
    LIBSSH2_CHANNEL *chan = libssh2_channel_open_session(sess);
    libssh2_channel_exec(chan, cmd);
    /* drain */ while (libssh2_channel_read(chan, NULL, 0) > 0);
    libssh2_channel_close(chan);
    libssh2_channel_free(chan);
    libssh2_session_disconnect(sess, "bye");
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
    LIBSSH2_CHANNEL *chan = libssh2_channel_open_session(sess);
    libssh2_channel_exec(chan, cmd);
    size_t pos = 0;
    while (1) {
        ssize_t n = libssh2_channel_read(chan, out + pos, outlen - pos - 1);
        if (n <= 0) break;
        pos += n;
    }
    out[pos] = '\0';
    libssh2_channel_close(chan);
    libssh2_channel_free(chan);
    libssh2_session_disconnect(sess, "bye");
    libssh2_session_free(sess);
    libssh2_exit();
    close(sock);
    return 0;
}
