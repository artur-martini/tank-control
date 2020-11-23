#ifndef CLIENT_H
#define CLIENT_H

#define BUFFSIZE 32

typedef struct{
   int code;
   int value; 
}Command;

void Die(char *mess);

void myClient(char serverIP[10], char serverPort[10], char message[BUFFSIZE]);

Command getAnswer();

#endif