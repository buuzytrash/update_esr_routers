#include "uploader.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"

void log_message(const char *format, ...) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    va_list args;
    va_start(args, format);
    vfprintf(log, format, args);
    va_end(args);

    fprintf(log, "\n");
    fclose(log);
}

int load_config_from_ini(UploadConfig *cfg, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "ip = %[0-9.:]", cfg->router_ip)) continue;
        if (sscanf(line, "user = %[a-zA-Z0-9]", cfg->router_user)) continue;
        if (sscanf(line, "pass = %[^\n]", cfg->router_pass)) continue;
        if (sscanf(line, "firmware = %[^\n]", cfg->firmware_file)) continue;
        if (strstr(line, "dry_run = true")) cfg->dry_run = 1;
    }
    fclose(f);
    return 1;
}

int upload_firmware(const UploadConfig *cfg) {
    char cmd[512];
    log_message("[*] Uploading firmware: %s", cfg->firmware_file);

    if (access(cfg->firmware_file, F_OK) != 0) {
        fprintf(stderr, "[!] Firmware file not found: %s\n",
                cfg->firmware_file);
        log_message("[!] File not found: %s", cfg->firmware_file);
        return 1;
    }

    if (cfg->dry_run) {
        printf("[dry-run] sudo cp %s /srv/tftp/ && chmod 644 ...\n",
               cfg->firmware_file);
        log_message("[dry-run] copy %s", cfg->firmware_file);
        return 0;
    }

    snprintf(cmd, sizeof(cmd),
             "sudo cp %s /srv/tftp/ && sudo chmod 644 /srv/tftp/%s",
             cfg->firmware_file, cfg->firmware_file);

    log_message("Executing: %s", cmd);
    return system(cmd);
}

int install_firmware(const UploadConfig *cfg) {
    char cmd1[512], cmd2[256], cmd3[256];

    snprintf(cmd1, sizeof(cmd1),
             "sshpass -p '%s' ssh %s@%s \"copy tftp://192.168.56.1:/%s "
             "system:firmware\"",
             cfg->router_pass, cfg->router_user, cfg->router_ip,
             cfg->firmware_file);

    snprintf(cmd2, sizeof(cmd2),
             "sshpass -p '%s' ssh %s@%s \"boot system image-2\"",
             cfg->router_pass, cfg->router_user, cfg->router_ip);

    snprintf(cmd3, sizeof(cmd3), "sshpass -p '%s' ssh %s@%s \"reload system\"",
             cfg->router_pass, cfg->router_user, cfg->router_ip);

    if (cfg->dry_run) {
        printf("[dry-run] %s\n[dry-run] sleep 30\n[dry-run] %s\n[dry-run] %s\n",
               cmd1, cmd2, cmd3);
        log_message("[dry-run] %s", cmd1);
        log_message("[dry-run] %s", cmd2);
        log_message("[dry-run] %s", cmd3);
        return 0;
    }

    log_message("[*] Executing install phase");
    if (system(cmd1) != 0) return 1;
    sleep(30);
    if (system(cmd2) != 0) return 2;
    return system(cmd3);
}

void check_firmware_ready(const UploadConfig *cfg) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "sshpass -p '%s' ssh %s@%s \"show bootvar\"",
             cfg->router_pass, cfg->router_user, cfg->router_ip);
    printf("[*] Verifying firmware status:\n");
    log_message("[*] Checking show bootvar");
    system(cmd);
}

void check_cluster_sync(const UploadConfig *cfg) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "sshpass -p '%s' ssh %s@%s \"show cluster sync status\"",
             cfg->router_pass, cfg->router_user, cfg->router_ip);
    printf("[*] Verifying cluster synchronization:\n");
    log_message("[*] Checking show cluster sync status");
    system(cmd);
}