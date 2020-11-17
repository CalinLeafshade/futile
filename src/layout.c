
#include "log.h"
#include "types.h"
#include "monitor.h"
#include <X11/Xlib.h>

void layoutMonitor(Monitor* monitor, Display* display, int borderWidth) {

  int sw = monitor->w;
	int sh = monitor->h;

  int index = 0;
  int clientCount = countClients(monitor);
  Client* c = monitor->clients;

  while (c) {
  
    if (clientCount == 1) {
      // Just maximimize the window
      XMoveResizeWindow(
					display, 
					c->window, 
					monitor->x,
					monitor->y,
					sw - borderWidth * 2,
					sh - borderWidth * 2
			);
    }
    else if (index == 0) {
      // Top of the stack
      XMoveResizeWindow(
					display, 
					c->window, 
					monitor->x, 
					monitor->y, 
					sw / 2 - (borderWidth * 2), 
					sh - borderWidth * 2
			);
    }
    else {
      int windowHeight = sh / (clientCount - 1);
      XMoveResizeWindow(
          display, 
          c->window, 
          c->monitor->x + sw / 2, 
          c->monitor->y + windowHeight * (index - 1),
          sw / 2 - (borderWidth * 2), 
          windowHeight - borderWidth * 2
      );
    }

    index ++;
    c = c->next;
  }

}

