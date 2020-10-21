#ifndef CLIENT_H
#define CLIENT_H

#define BUFFSIZE 32

void Die(char *mess);

void myClient(char serverIP[10], char message[BUFFSIZE], char serverPort[10]);

#endif