#ifndef CLIENT_H
#define CLIENT_H

#define BUFFSIZE 32

typedef struct{
   int code;
   int value; 
}Command;

void Die(char *mess);

void myClient(char serverIP[10], char message[BUFFSIZE], char serverPort[10]);

Command getAnswer();

#endif