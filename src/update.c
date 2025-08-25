#include "update.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "ssh.h"
#include "ui.h"
#include "utils.h"

#define REBOOT_DELAY 600

static size_t g_idx;

static void on_output_cb(const char *line) { ui_log_append(line); }
static void on_progress_cb(int pct) {
    int tleft = (100 - pct) * REBOOT_DELAY / 100;
    ui_update_detail(g_idx, pct, tleft);
}

static int parse_inactive_image(const char *bootvar_output) {
    char line[256];
    const char *p = bootvar_output;
    while (*p) {
        int i = 0;
        while (*p && *p != '\n' && i < (int)sizeof(line) - 1) {
            line[i++] = *p++;
        }
        line[i] = '\0';
        if (*p == '\n') ++p;
        int img;
        if (sscanf(line, "%d %*s %*s %*s Not Active", &img) == 1) {
            return img;
        }
    }
    return -1;
}

int update_router(const char *ip, const Config *c, size_t idx) {
    g_idx = idx;
    char cmd[512];

    // 1) Copy firmware
    snprintf(cmd, sizeof(cmd), "copy tftp://%s:/%s system:firmware",
             c->tftp_server_ip, c->firmware_file);
    cmd[strcspn(cmd, "\r\n")] = '\0';
    ui_log_append("\n=== Copying firmware ===");
    ui_log_append(cmd);
    if (ssh_execute_stream(ip, c, cmd, on_output_cb, on_progress_cb) != 0) {
        ui_log_append("✖ Firmware copy failed");
        return -1;
    }
    ui_log_append("✔ Firmware copy complete");

    // 2) Determine inactive image
    ui_log_append("\n=== Determining inactive image ===");
    char bootvar[1024] = {0};
    if (ssh_execute_capture(ip, c, "show bootvar", bootvar, sizeof(bootvar)) !=
        0) {
        ui_log_append("✖ Failed to retrieve bootvar");
        return -1;
    }
    ui_log_append(bootvar);
    int inactive = parse_inactive_image(bootvar);
    if (inactive < 1) {
        ui_log_append("✖ Could not parse inactive image");
        return -1;
    }
    {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), "Selected image-%d", inactive);
        ui_log_append(tmp);
    }

    // 3) Set boot image
    ui_log_append("\n=== Setting boot image ===");
    snprintf(cmd, sizeof(cmd), "boot system image-%d", inactive);
    cmd[strcspn(cmd, "\r\n")] = '\0';
    ui_log_append(cmd);
    if (ssh_execute_stream(ip, c, cmd, on_output_cb, NULL) != 0) {
        ui_log_append("✖ Boot system command failed");
        return -1;
    }
    ui_log_append("✔ Boot image set successfully");

    // 4) Reload
    ui_log_append("\n=== Reloading device ===");
    ssh_execute(ip, c, "reload system");

    // 5) Wait for device down/up with timeouts and quit key
    ui_log_append("🔄 Waiting for device to go down...");
    nodelay(stdscr, TRUE);
    int ch, tries = 60;
    while (tries-- > 0 && pingable(ip)) {
        sleep_sec(5);
        ch = getch();
        if (ch == 'q' || ch == 'Q') exit(0);
    }
    if (tries <= 0) {
        ui_log_append("✖ Device never went down");
        nodelay(stdscr, FALSE);
        return -1;
    }
    ui_log_append("🔄 Waiting for device to come back up...");
    tries = 120;
    while (tries-- > 0 && !pingable(ip)) {
        sleep_sec(5);
        ch = getch();
        if (ch == 'q' || ch == 'Q') exit(0);
    }
    nodelay(stdscr, FALSE);
    if (tries <= 0) {
        ui_log_append("✖ Device never came back up");
        return -1;
    }
    ui_log_append("✅ Device is online");

    // 6) Verify bootvar
    ui_log_append("\n=== Verifying bootvar ===");
    memset(bootvar, 0, sizeof(bootvar));
    if (ssh_execute_capture(ip, c, "show bootvar", bootvar, sizeof(bootvar)) ==
        0) {
        ui_log_append(bootvar);
    } else {
        ui_log_append("✖ Failed to verify bootvar");
    }

    // 7) Show version
    ui_log_append("\n=== Showing version ===");
    {
        char ver[256] = {0};
        if (ssh_execute_capture(ip, c, "show version", ver, sizeof(ver)) == 0) {
            ui_log_append(ver);
        } else {
            ui_log_append("✖ Failed to show version");
        }
    }

    ui_log_append("\n=== Update completed for ");
    ui_log_append(ip);
    return 0;
}
