#ifndef DECODER_H
#define DECODER_H

#define BUFFSIZE 32

/* define o tamanho máximo da mensagem recebida*/
#define MESSAGE_SIZE 32

typedef struct{
   int code;
   int value; 
}Command;

Command getClientCommand(char buffer[BUFFSIZE]);

#endif