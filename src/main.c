#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "logging.h"
#include "update.h"

int main(int argc, char** argv) {
    const char* cfg = argc > 1 ? argv[1] : "config.ini";
    Config c;
    if (load_config(cfg, &c) != 0) {
        fprintf(stderr, "Bad cfg\n");
        return 1;
    }
    init_logging("esr.log");
    for (size_t i = 0; i < c.router_count; ++i)
        update_router(c.router_ips[i], &c);
    free_config(&c);
    return 0;
}