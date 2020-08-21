#pragma once

#include <stdlib.h>

#include <arpa/inet.h>
#define GAMEOVER_TYPE 4
#define BUFSZ 1024
#define MAX_CONNECTIONS 10

void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(int proto, const char *portstr,
                         struct sockaddr_storage *storage);
