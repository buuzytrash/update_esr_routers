#include <libssh2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "logging.h"
#include "ui.h"
#include "update.h"
#include "utils.h"

int main(int argc, char **argv) {
    Config cfg;
    if (load_config(argc > 1 ? argv[1] : "config.ini", &cfg) != 0) {
        fprintf(stderr, "Failed to load config\n");
        return 1;
    }

    init_logging("esr_ui.log");
    log_msg(LOG_INFO, "Loaded %zu routers", cfg.router_count);

    ui_init();
    ui_draw_layout();

    // Initial router list
    const char **status = malloc(cfg.router_count * sizeof(char *));
    for (size_t i = 0; i < cfg.router_count; ++i) status[i] = "PENDING";
    ui_update_router_list((const char **)cfg.router_ips, cfg.router_count,
                          status);

    // Select method
    const char *methods[] = {"tftp", "scp"};
    int m = ui_select_method(methods, 2);
    cfg.method = strdup(methods[m]);
    log_msg(LOG_INFO, "Method %s", cfg.method);

    // Select routers
    size_t selc;
    int *sel = ui_select_routers((const char **)cfg.router_ips,
                                 cfg.router_count, &selc);
    if (selc == 0) {
        free(sel);
        selc = cfg.router_count;
        sel = malloc(selc * sizeof(int));
        for (size_t i = 0; i < selc; ++i) sel[i] = i;
    }

    // Run updates
    for (size_t i = 0; i < selc; ++i) {
        size_t idx = sel[i];
        update_router(cfg.router_ips[idx], &cfg, idx);
        status[idx] = "OK";
        ui_update_router_list((const char **)cfg.router_ips, cfg.router_count,
                              status);
    }

    ui_deinit();
    free(sel);
    free(status);
    free_config(&cfg);
    libssh2_exit();
    return 0;
}
