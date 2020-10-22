#ifndef SERVER_H
#define SERVER_H


void Die(char *mess);

void HandleClient(int sock);

void *Server(void *value);

void *Tank(void *value);

int getCommandCode();

int getCommandValue();

#endif