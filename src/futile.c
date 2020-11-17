#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

#include "types.h"
#include "client.h"
#include "monitor.h"
#include "layout.h"
#include "log.h"

enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */

static Display *display;
static Monitor *monitors, *activeMonitor;
static int screen;
static int running = 1;
static Window root;
static Atom wmatom[WMLast];

static int borderWidth = 2;
static int (*xerrorxlib)(Display *, XErrorEvent *);

static void focus(Window w);
static void die(const char *msg);
static int sendevent(Window w, Atom proto);

unsigned long _RGB(int r,int g, int b)
{
    return b + (g<<8) + (r<<16);
}

void die(const char *msg) {
  printf("%s", msg);
  exit(1);
}

void quit(void) {
  running = 0;
}

void cleanup(void) {

}

void adopt(Window w, XWindowAttributes *attrib) {

  XWindowChanges wc;

  XSelectInput(display, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
  wc.border_width = borderWidth;
  XConfigureWindow(display, w, CWBorderWidth, &wc);
  XSetWindowBorder(display, w, _RGB(255,0,0));
  Client *c = createClient(w);
  attachClient(c, activeMonitor);
  layoutMonitor(activeMonitor, display, borderWidth);
  XMapWindow(display, w);
}

void unadopt(Window w) {

  Client *c = findWindow(w,monitors);

  if (c != NULL) {
    detachClient(c);
    layoutMonitor(c->monitor, display, borderWidth);
  }

}

void mapRequest(XEvent *e) {
  static XWindowAttributes wa;
  XMapRequestEvent *ev = &e->xmaprequest;

  if (!XGetWindowAttributes(display, ev->window, &wa))
    return;
  if (wa.override_redirect)
    return;
  adopt(ev->window, &wa);
}

int xerror(Display *dpy, XErrorEvent *ee)
{
  if (ee->error_code == BadWindow) {
    return 0;
  } 
  return xerrorxlib(dpy, ee); /* may call exit */
}

void destroyNotify(XEvent *e) {
  XDestroyWindowEvent *ev = &e->xdestroywindow;

  unadopt(ev->window);

}

void updateBorders() {

  Monitor *head = monitors;
  unsigned long activeColor = _RGB(255,0,0);
  unsigned long inactiveColor = _RGB(60,60,60);

  while (head != NULL) {
    Client* clientHead = head->clients;
    while(clientHead != NULL) {
      int isActive = clientHead->monitor->activeClient == clientHead;
      XSetWindowBorder(display, clientHead->window, isActive ? activeColor : inactiveColor);
      clientHead = clientHead->next;
    }

    head = head->next;
  }

}

void focus (Window w) {

  Client *c = findWindow(w, monitors);

  if (c != NULL) {
    activeMonitor = c->monitor;
    activeMonitor->activeClient = c;
  }

  updateBorders();

  if (w == -1) {
    w = root;
  }

  logger(2, "Setting input focus to %d", w);
  
  XSetInputFocus(display, w, RevertToPointerRoot, CurrentTime);
  sendevent(w, wmatom[WMTakeFocus]);
}

void enterNotify(XEvent *e)
{
  XCrossingEvent *ev = &e->xcrossing;

  if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root) {
    return;
  }

  focus(ev->window);
}

int sendevent(Window w, Atom proto)
{
  int n;
  Atom *protocols;
  int exists = 0;
  XEvent ev;

  if (XGetWMProtocols(display, w, &protocols, &n)) {
    while (!exists && n--)
      exists = protocols[n] == proto;
    XFree(protocols);
  }
  if (exists) {
    ev.type = ClientMessage;
    ev.xclient.window = w;
    ev.xclient.message_type = wmatom[WMProtocols];
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = proto;
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(display, w, False, NoEventMask, &ev);
  }
  return exists;
}

void initialize(void) {

  XSetWindowAttributes wa;

  xerrorxlib = XSetErrorHandler(xerror);

  monitors = activeMonitor = getMonitors(display);
  screen = DefaultScreen(display);
  root = RootWindow(display, screen);

  wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
    |ButtonPressMask|PointerMotionMask|EnterWindowMask
    |LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;

  XChangeWindowAttributes(display, root, CWEventMask|CWCursor, &wa);
  XSelectInput(display, root, wa.event_mask);

  wmatom[WMProtocols] = XInternAtom(display, "WM_PROTOCOLS", False);
  wmatom[WMDelete] = XInternAtom(display, "WM_DELETE_WINDOW", False);
  wmatom[WMState] = XInternAtom(display, "WM_STATE", False);
  wmatom[WMTakeFocus] = XInternAtom(display, "WM_TAKE_FOCUS", False);

  printf("Screen: %d\n", screen);
  printf("Root: %lu\n", root);

}

int read_line(int fd, char *buf, size_t bufsiz)
{
  size_t i = 0;
  char c = '\0';

  do {
    if (read(fd, &c, sizeof(char)) != sizeof(char)) {
      return -1;
    }
    buf[i++] = c;
  } while (c != '\n' && i < bufsiz);

  buf[i - 1] = '\0'; /* eliminates '\n' */
  return 0;
}

int xerrordummy(Display *display, XErrorEvent *ee)
{
  return 0;
}

void closeClient(Client *c) {
  Window w = c->window;
  if (!sendevent(w, wmatom[WMDelete])) { 
    XGrabServer(display);
    XSetErrorHandler(xerrordummy);
    XSetCloseDownMode(display, DestroyAll);
    XKillClient(display, w);
    XSync(display, False);
    XSetErrorHandler(xerror);
    XUngrabServer(display);
    unadopt(w);
  }
}


void processCommand(char *buf) {
  printf("Received Command: %.80s\n", buf);

  if (strncmp("quit", buf, strlen("quit")) == 0) {
    quit();
  }
  else if (strncmp("close", buf, strlen("close")) == 0) {
    closeClient(activeMonitor->activeClient);
  }
  else if (strncmp("bump", buf, strlen("bump")) == 0) {
    Client*c = activeMonitor->activeClient;
    detachClient(c);
    attachClient(c, activeMonitor);
    layoutMonitor(activeMonitor, display, borderWidth);
    focus(c->window);
  }
  else if (strncmp("next", buf, strlen("next")) == 0) {
    Client* c = activeMonitor->activeClient;
    if (c != NULL) {
      if (c->next != NULL) {
        focus(c->next->window);
      }
      else {
        focus(activeMonitor->clients->window);
      }
    }
  }
  else if (strncmp("prev", buf, strlen("prev")) == 0) {
    Client* c = activeMonitor->activeClient;
    Client* prev = activeMonitor->clients;

    while(prev != NULL && prev->next != c) {
      prev = prev->next;
    }

    if (prev != NULL) {
      focus(prev->window);
    }
  }
}

long getstate(Window w)
{
  int format;
  long result = -1;
  unsigned char *p = NULL;
  unsigned long n, extra;
  Atom real;

  if (XGetWindowProperty(display, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
                        &real, &format, &n, &extra, (unsigned char **)&p) != Success)
    return -1;
  if (n != 0)
    result = *p;
  XFree(p);
  return result;
}

void claimCurrentWindows() {
  unsigned int i, num;
  Window d1, d2, *wins = NULL;
  XWindowAttributes wa;

  if (XQueryTree(display, root, &d1, &d2, &wins, &num)) {
    for (i = 0; i < num; i++) {
      if (!XGetWindowAttributes(display, wins[i], &wa) 
      || wa.override_redirect || XGetTransientForHint(display, wins[i], &d1)) {
        continue;
      }
      if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState) {
        adopt(wins[i], &wa);
      }
    }
    for (i = 0; i < num; i++) { /* now the transients */
      if (!XGetWindowAttributes(display, wins[i], &wa))
        continue;
      if (XGetTransientForHint(display, wins[i], &d1)
      && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
        adopt(wins[i], &wa);
    }
    if (wins)
      XFree(wins);

  }
}

void motionNotify(XEvent *e) {
  
  XMotionEvent *ev = &e->xmotion;

  if (ev->window != root) {
    return;
  }

  logger(2, "%d, %d", ev->x, ev->y);

  Monitor* m = pointToMonitor(ev->x_root, ev->y_root, monitors);

  activeMonitor = m;
  if (m->activeClient == NULL) {
    focus(-1);
  }
  else {
    focus(m->activeClient->window);
  }

}


void mainLoop(void) {

    char* fifo = "/tmp/futile"; 
  
    mkfifo(fifo, 0666); 

  int fd1 = open(fifo, O_RDONLY | O_NONBLOCK);
  char buf[80];
  XEvent ev;

  XSync(display, False);
  while (running) {

    int recv = read_line(fd1, buf, sizeof(buf));
    if (recv > -1) {
      processCommand(buf);
    }

    while (XPending(display)) {
      XNextEvent(display, &ev);
      if (ev.type == MapRequest) {
        mapRequest(&ev);
      }
      else if (ev.type == DestroyNotify) {
        destroyNotify(&ev);
      }
      else if (ev.type == EnterNotify) {
        enterNotify(&ev);
      }
      else if (ev.type == MotionNotify) {
        motionNotify(&ev);
      }
      printf("Event: %d\n", ev.type);
    }

    sleep(0);
  }

}

int main(int argc, char *argv[])
{


  display = XOpenDisplay(NULL);

  if (!display) {
    die("Cannot open display");
  }

  initialize();
  claimCurrentWindows();
  mainLoop();
  cleanup();
  XCloseDisplay(display);
  return EXIT_SUCCESS;
}
