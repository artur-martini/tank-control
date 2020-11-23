#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "decoder.h"



Command getServerAnswer(char *buffer){
    Command server_command;
	server_command.code = 0;
    server_command.value = 0;

	char message_value[10];
	char message[MESSAGE_SIZE];

    int i,j = 0;

	/* clear message*/
	i = 0;
	while(message[i] != '\0'){
		message[i] = 0;
		i++;
	}

	/* copia mensagem até encontrar um "!"*/
	i = 0;
	while(buffer[i] != '\0'){
		message[i] = buffer[i];
		i++;
	}
	//message[i] = '!';
	// message[i] = buffer[i]; /* copia o "!" para a mensagem também */
	
	/* procura o comando na mensagem*/
	// printf("Client Message = %s \n", message);
	printf("Server answer-> %s, message -> %s\n", buffer, message);

	i = 0;
	j = 0;
	if (strstr(message, "Open") != NULL){
		server_command.code = 1;
		// command_value = getCommandValue("OpenValve#100!");		
		while(message[i] != '#') i++;
		i++; /* pula o "#" */
		while(message[i] != '!'){
			message_value[j] = message[i];
			i++;
			j++;
		}
		server_command.value = atoi(message_value);
	}
	else if(strstr(message, "Close") != NULL){
		server_command.code = 2;
		while(message[i] != '#') i++;
		i++; /* pula o "#" */
		while(message[i] != '!'){
			message_value[j] = message[i];
			i++;
			j++;
		}
		server_command.value = atoi(message_value);
	}
	else if(strstr(message, "Level") != NULL){
		server_command.code = 3;
		while(message[i] != '#') i++;
		i++; /* pula o "#" */
		while(message[i] != '!'){
			message_value[j] = message[i];
			i++;
			j++;
		}
		server_command.value = atoi(message_value);
	}
	else if(strstr(message, "Comm#OK!") != NULL){
		server_command.code = 4;
	}
	else if(strstr(message, "Max") != NULL){
		server_command.code = 5;
		while(message[i] != '#') i++;
		i++; /* pula o "#" */
		while(message[i] != '!'){
			message_value[j] = message[i];
			i++;
			j++;
		}
		server_command.value = atoi(message_value);
	}
	else if(strstr(message, "Start#OK!") != NULL){
		server_command.code = 6;
		server_command.value = -1;	
	}
	else{ 
		printf("COMANDO INVÁLIDO\n");
		server_command.code = 0;
		server_command.value = -1;
	}
	
	/* limita o valor máximo e mínimo dos comandos*/
	if(server_command.value > 100){
		server_command.value = 100;
	}
	if(server_command.value < 0){
		server_command.value = 0;
	}

	/* lista de comandos que o servidor deve reconhecer
	
	1 - OpenValve, sintaxe: OpenValve#<value>!
	2 - CloseVale, sintaxe: CloseValve#<value>!
	3 - GetLevel, sintaxe: GetLevel!
	4 - CommTest, sintaxe: CommTest!
	5 - SetMax, sintaxe: SetMax#<value>!
	6 - Start, sintaxe: Start!	
	
	*/
    return server_command;
}