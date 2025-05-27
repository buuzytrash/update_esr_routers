#include <stdio.h>

#include "uploader.h"

int main() {
    printf("Firmware Upload Utility for vESR\n");

    if (upload_firmware() != 0) {
        fprintf(stderr, "[!] Firmware upload failed.\n");
        return 1;
    }

    if (install_firmware() != 0) {
        fprintf(stderr, "[!] Firmware installation failed.\n");
        return 2;
    }

    printf("[+] Firmware update completed. Please wait for reboot.\n");
    return 0;
}
