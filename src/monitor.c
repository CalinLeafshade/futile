#include <X11/extensions/Xinerama.h>
#include <stdlib.h>
#include "monitor.h"
#include "log.h"

Monitor* createMonitor(int i, int x, int y, int w, int h) {
  
  Monitor *m;

  m = calloc(1, sizeof(Monitor));
  m->index = i;
  m->x = x;
  m->y = y;
  m->w = w;
  m->h = h;
  m->activeClient = NULL;
  m->clients = NULL;
  m->next = NULL;

  logger(2, "Creating Monitor: %d %d %d %d", x, y, w ,h);

  return m;

}

int countClients(Monitor* monitor) {
  
  Client* temp = monitor->clients;

  int count = 0;

  while (temp != NULL) {
    temp = temp->next;
    count++;
  }

  return count;

}

void attachClient(Client* client, Monitor* monitor) {
  logger(2, "Attaching client to monitor %d", monitor->index);
  client->next = monitor->clients;
  client->monitor = monitor;
  monitor->clients = client;
  if (monitor->activeClient == NULL) {
    monitor->activeClient = client;
  }
}

void detachClient(Client* client) {

  Client** head;

  for(head = &client->monitor->clients; *head && *head != client; head = &(*head)->next);
  *head = client->next;

}

Client* findWindow(Window window, Monitor* monitors) {

  Monitor *head = monitors;

  while (head != NULL) {
    Client* clientHead = head->clients;
    while(clientHead != NULL) {
      if (clientHead->window == window) {
        return clientHead;
      }
      clientHead = clientHead->next;
    }

    head = head->next;
  }

  return NULL;

}

Monitor* pointToMonitor(int x, int y, Monitor* monitors) {


  Monitor *head = monitors;

  while (head != NULL) {
    if (head->x <= x && head->y <= y && head->x + head->w > x && head->y + head->h > y) {
      return head;
    }

    head = head->next;
  }

  return NULL;

}

Monitor* getMonitors(Display *display) {

  int monitorCount;
  Monitor* monitors = NULL;
  Monitor* next = NULL;

  XineramaScreenInfo *info = XineramaQueryScreens(display, &monitorCount);

  for(int i = 0; i < monitorCount; i++) {
    Monitor* m = createMonitor(i, info[i].x_org, info[i].y_org, info[i].width, info[i].height);
    if (i == 0) {
      monitors = m;
    }
    else {
      next->next = m;
    }
    next = m;
  }

  return monitors;

}
