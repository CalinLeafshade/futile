#include <stdlib.h>

#include "client.h"

Client* createClient(Window w) {
  
  Client* client;

  client = calloc(1, sizeof(Client));

  client->next = NULL;
  client->monitor = NULL;
  client->window = w;

  return client;

}
