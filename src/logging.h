#ifndef LOGGING_H
#define LOGGING_H

#define FATAL 0
#define ERROR 1
#define WARN  2
#define INFO  3
#define DEBUG 4
#define TRACE 5

extern int log_level;

const char* level_string(int level);
void parse_options(int argc, char **argv);
void log_msg(int level, const char *format, ...);

#endif
