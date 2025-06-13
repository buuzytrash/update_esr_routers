#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "config.h"
#include "uploader.h"

void prompt_hidden(const char *prompt, char *buffer, size_t size) {
    struct termios oldt, newt;
    printf("%s", prompt);
    fflush(stdout);
    tcgetattr(fileno(stdin), &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &newt);
    fgets(buffer, size, stdin);
    tcsetattr(fileno(stdin), TCSANOW, &oldt);
    buffer[strcspn(buffer, "\n")] = '\0';
    printf("\n");
}

int main(int argc, char *argv[]) {
    UploadConfig cfg = {0};

    if (argc > 1 && strcmp(argv[1], "--dry-run") == 0) cfg.dry_run = 1;

    if (!load_config_from_ini(&cfg, CONFIG_FILE)) {
        printf("[!] Config file not found, entering interactive mode.\n");

        printf("Router IP: ");
        fgets(cfg.router_ip, sizeof(cfg.router_ip), stdin);
        cfg.router_ip[strcspn(cfg.router_ip, "\n")] = '\0';

        printf("Username: ");
        fgets(cfg.router_user, sizeof(cfg.router_user), stdin);
        cfg.router_user[strcspn(cfg.router_user, "\n")] = '\0';

        prompt_hidden("Password: ", cfg.router_pass, sizeof(cfg.router_pass));

        printf("Firmware file: ");
        fgets(cfg.firmware_file, sizeof(cfg.firmware_file), stdin);
        cfg.firmware_file[strcspn(cfg.firmware_file, "\n")] = '\0';
    }

    log_message("=== Firmware Uploader started ===");

    if (upload_firmware(&cfg) != 0) {
        fprintf(stderr, "[!] Firmware upload failed.\n");
        log_message("[!] Upload failed.");
        return 1;
    }

    if (install_firmware(&cfg) != 0) {
        fprintf(stderr, "[!] Firmware installation failed.\n");
        log_message("[!] Install failed.");
        return 2;
    }

    check_firmware_ready(&cfg);
    check_cluster_sync(&cfg);
    log_message("[+] Firmware update complete.\n");

    printf("[+] Firmware update process complete.\n");
    return 0;
}