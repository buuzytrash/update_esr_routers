#include "uploader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"

int upload_firmware() {
    char cmd[512];

    printf("[*] Copying firmware to TFTP directory...\n");

    if (access(LOCAL_FIRMWARE_PATH, F_OK) != 0) {
        fprintf(stderr, "[!] Firmware file not found: %s\n",
                LOCAL_FIRMWARE_PATH);
        return 1;
    }

    snprintf(cmd, sizeof(cmd), "sudo cp %s %s && sudo chmod 644 %s",
             LOCAL_FIRMWARE_PATH, TFTP_SERVER_PATH, TFTP_SERVER_PATH);

    return system(cmd);
}

int install_firmware() {
    char cmd1[512];
    char cmd2[256];
    char cmd3[256];

    snprintf(cmd1, sizeof(cmd1),
             "sshpass -p '%s' ssh %s@%s \"copy %s%s:/%s system:firmware\"",
             ROUTER_PASS, ROUTER_USER, ROUTER_IP, TFTP_NAME, HOST_IP,
             LOCAL_FIRMWARE_PATH);

    snprintf(cmd2, sizeof(cmd2),
             "sshpass -p '%s' ssh %s@%s \"boot system image-2\"", ROUTER_PASS,
             ROUTER_USER, ROUTER_IP);

    snprintf(cmd3, sizeof(cmd3), "sshpass -p '%s' ssh %s@%s \"reload system\"",
             ROUTER_PASS, ROUTER_USER, ROUTER_IP);

    printf("[*] Initiating firmware installation via TFTP...\n");
    if (system(cmd1) != 0) return 1;

    printf("[*] Setting boot image to image-2...\n");
    if (system(cmd2) != 0) return 2;

    printf("[*] Rebooting the router...\n");
    return system(cmd3);
}
