
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* defaultColor = "\033[0m";
static const char* colors[] = {
  "\033[91m",
  "\033[93m",
  "\033[0m"
};

void logger(int level, const char* fmt, ...) {

  
  printf("%s", colors[level]);
  va_list args;

  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);

  printf("%s\n", defaultColor);

}
