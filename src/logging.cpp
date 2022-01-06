#include <stdio.h>
#include <unistd.h>

#include <stdarg.h>

#include "logging.h"

int log_level;

const char* level_string(int level)
{
   switch (level) {
   case FATAL:
      return "FATAL";

   case ERROR:
      return "ERROR";

   case WARN:
      return "WARN";

   case INFO:
      return "INFO";

   case DEBUG:
      return "DEBUG";

   case TRACE:
      return "TRACE";

   default:
      if (level > TRACE)
	 return "TRACE";
      return "[bad log level]";
   }
}


void parse_options(int argc, char **argv)
{
   log_level = WARN;
   int option;
   
   while((option = getopt(argc, argv, "vq")) != -1) {
      switch(option) {
      case 'q':
	 log_level -= 1;
	 break;

      case 'v':
	 log_level += 1;
	 break;
      }
   }
}


void log_msg(int level, const char *format, ...)
{
   if (log_level >= level) {
      va_list args;
      va_start(args, format);
      fprintf(stderr, "[%s] ", level_string(level));
      vfprintf(stderr, format, args);
      fprintf(stderr, "\n");
      va_end(args);
   }
}
