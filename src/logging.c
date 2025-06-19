#include "logging.h"

#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
static FILE *log_file = NULL;
void init_logging(const char *f) {
    log_file = fopen(f, "a");
    if (!log_file) {
        perror("fopen");
        exit(1);
    }
}
void log_msg(LogLevel lvl, const char *fmt, ...) {
    if (!log_file) return;
    const char *lvlstr[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d %s: ", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
            lvlstr[lvl]);
    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);
    fprintf(log_file, "\n");
    fflush(log_file);
}