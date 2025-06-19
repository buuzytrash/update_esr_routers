#ifndef SSH_H
#define SSH_H

#include <stddef.h>

#include "config.h"
int ssh_execute_capture(const char *ip, const Config *cfg, const char *cmd,
                        char *out, size_t outlen);

int ssh_execute(const char *ip, const Config *cfg, const char *cmd);
#endif  // SSH_H