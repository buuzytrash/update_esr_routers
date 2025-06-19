#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR } LogLevel;
void init_logging(const char *logfile);
void log_msg(LogLevel level, const char *fmt, ...);

#endif // LOGGING_H