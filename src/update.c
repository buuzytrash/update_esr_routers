#include "update.h"

#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "ssh.h"
#include "utils.h"

int update_router(const char *ip, const Config *c) {
    log_msg(LOG_INFO, "Starting update %s via %s", ip, c->method);

    char cmd[512], buf[512] = {0};
    if (strcmp(c->method, "tftp") == 0) {
        snprintf(cmd, sizeof(cmd), "copy tftp://%s:/%s system:firmware",
                 c->tftp_server_ip, c->firmware_file);
    } else if (strcmp(c->method, "scp") == 0) {
        snprintf(cmd, sizeof(cmd), "copy scp://%s:%s@%s:/%s system:firmware",
                 c->scp_user, c->scp_pass, c->scp_server_ip, c->firmware_file);
    } else {
        log_msg(LOG_ERROR, "Unknown method '%s'", c->method);
        return -1;
    }

    // выполняем команду и ждем 100%
    if (ssh_execute_capture(ip, c, cmd, buf, sizeof(buf)) != 0) {
        log_msg(LOG_ERROR, "Copy failed on %s: %s", ip, buf);
        return -1;
    }
    if (!strstr(buf, "100%")) {
        log_msg(LOG_WARN, "Copy output unexpected on %s: %s", ip, buf);
    }
    if (ssh_execute(ip, c, "reload") != 0) return -1;
    /* wait reboot */
    while (pingable(ip)) sleep_sec(1);
    while (!pingable(ip)) sleep_sec(1);
    char ver[128];
    ssh_execute_capture(ip, c, "show version", ver, sizeof(ver));
    log_msg(LOG_INFO, "New version %s: %s", ip, ver);
    log_msg(LOG_INFO, "Done %s", ip);
    return 0;
}