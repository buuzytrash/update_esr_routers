#ifndef SSH_H
#define SSH_H
#include "config.h"
int ssh_execute(const char *ip, const Config *cfg, const char *cmd);
int ssh_execute_capture(const char *ip, const Config *cfg, const char *cmd,
                        char *out, size_t outlen);
int ssh_execute_stream(const char *ip, const Config *cfg, const char *cmd,
                       void (*on_output)(const char *line),
                       void (*on_progress)(int percent));
#endif  // SSH_H