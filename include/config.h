#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

typedef struct {
    char **router_ips;
    size_t router_count;
    char *user;
    char *pass;
    char *ssh_key_path;
    char *firmware_file;
    char *tftp_server_ip;
} Config;

int load_config(const char *filename, Config *cfg);
void free_config(Config *cfg);

#endif  // CONFIG_H