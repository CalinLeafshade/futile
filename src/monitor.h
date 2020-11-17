#ifndef FUTILE_MONITOR_H
#define FUTILE_MONITOR_H

#include <X11/Xlib.h>
#include "types.h"

Monitor* createMonitor(int i, int x, int y, int w, int h);
Monitor* getMonitors(Display *display);
Monitor* pointToMonitor(int x, int y, Monitor* monitors);
Client* findWindow(Window window, Monitor* monitors);
int countClients(Monitor* monitor);
void attachClient(Client* client, Monitor* monitor);
void detachClient(Client* client);

#endif
