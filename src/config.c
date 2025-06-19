#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini.h"
static int config_handler(void* user, const char* section, const char* name,
                          const char* value) {
    Config* c = user;
    if (strcmp(section, "router") == 0 && strcmp(name, "ip") == 0) {
        char** tmp =
            realloc(c->router_ips, sizeof(char*) * (c->router_count + 1));
        if (!tmp) return 0;
        c->router_ips = tmp;
        c->router_ips[c->router_count++] = strdup(value);
    } else if (strcmp(section, "router") == 0 && strcmp(name, "user") == 0) {
        c->user = strdup(value);
    } else if (strcmp(section, "router") == 0 && strcmp(name, "pass") == 0) {
        c->pass = strdup(value);
    } else if (strcmp(section, "router") == 0 &&
               strcmp(name, "ssh_key_path") == 0) {
        c->ssh_key_path = strdup(value);
    } else if (strcmp(section, "firmware") == 0 && strcmp(name, "file") == 0) {
        c->firmware_file = strdup(value);
    } else if (strcmp(section, "tftp") == 0 && strcmp(name, "server_ip") == 0) {
        c->tftp_server_ip = strdup(value);
    } else if (strcmp(section, "scp") == 0 && strcmp(name, "user") == 0) {
        c->scp_user = strdup(value);
    } else if (strcmp(section, "scp") == 0 && strcmp(name, "pass") == 0) {
        c->scp_pass = strdup(value);
    } else if (strcmp(section, "scp") == 0 && strcmp(name, "server_ip") == 0) {
        c->scp_server_ip = strdup(value);
    } else if (strcmp(section, "update") == 0 && strcmp(name, "method") == 0) {
        c->method = strdup(value);
    }
    return 1;
}
int load_config(const char* fn, Config* c) {
    c->router_ips = NULL;
    c->router_count = 0;
    c->user = c->pass = c->ssh_key_path = NULL;
    c->firmware_file = c->tftp_server_ip = NULL;
    if (ini_parse(fn, config_handler, c) < 0) {
        fprintf(stderr, "Can't load %s\n", fn);
        return -1;
    }
    if (c->router_count == 0 || !c->user || (!c->pass && !c->ssh_key_path) ||
        !c->firmware_file || !c->tftp_server_ip) {
        fprintf(stderr, "Invalid config\n");
        return -1;
    }
    return 0;
}
void free_config(Config* c) {
    for (size_t i = 0; i < c->router_count; ++i) free(c->router_ips[i]);
    free(c->router_ips);
    free(c->user);
    free(c->pass);
    free(c->ssh_key_path);
    free(c->firmware_file);
    free(c->tftp_server_ip);
}