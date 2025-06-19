#include "update.h"

#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "ssh.h"
#include "utils.h"

int update_router(const char *ip, const Config *c) {
    log_msg(LOG_INFO, "Starting update %s", ip);
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "copy tftp://%s/%s system:firmware",
             c->tftp_server_ip, c->firmware_file);
    char buf[512];
    if (ssh_execute_capture(ip, c, cmd, buf, sizeof(buf)) != 0) return -1;
    if (!strstr(buf, "100%")) log_msg(LOG_WARN, "Copy output: %s", buf);
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