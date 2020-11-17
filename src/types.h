#ifndef FUTILE_TYPES_H
#define FUTILE_TYPES_H

#include <X11/X.h>

typedef struct Monitor Monitor;
typedef struct Client Client;

struct Monitor {
  int index;
  int x, y, w, h;
  Monitor* next;
  Client* clients;
  Client* activeClient;
};

struct Client {
  Client *next;
  Monitor *monitor;
  Window window;
};

#endif
