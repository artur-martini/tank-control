#ifndef READER_H
#define READER_H

#define BUFFSIZE 32

/* define o tamanho máximo da mensagem recebida*/
#define MESSAGE_SIZE 32

#include "data_types.h"

Command getClientCommand(char buffer[BUFFSIZE]);

#endif