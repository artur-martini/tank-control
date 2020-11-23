#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include "decoder.h"
#include "data_types.h"

#define MAXPENDING 5 /* max connection requests */
#define BUFFSIZE 32

/* define o tamanho máximo da mensagem recebida */
#define MESSAGE_SIZE 32

int _command_code = 0;
int _command_value = 0;
int level = 50;

void Die(char *mess) {perror(mess); exit(1); }

void HandleClient(int sock){
	char buffer[BUFFSIZE];
	int received = -1;

	/* limpa o buffer */
	int i;
	for(i = 0; i < BUFFSIZE; i++){
		buffer[i] = 0;
	}

	/* Receive message */
	if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0){
    	Die("Failed to receive initial bytes from client");
	}
	
	printf("buffer %s \n", buffer);
	Command server_command;

	server_command.code = 0;
	server_command.value = 0;

	server_command = getClientCommand(buffer);

	_command_code = server_command.code;
	_command_value = server_command.value;
	printf("==> Update code: %d, value: %d\n", _command_code, _command_value);

	/* Send bytes and check for more incoming data in loop */
	while (received > 0){
		/* Send back received data */
		char str[32];

		/* limpa char str */
		int i;
		for(i = 0; i < 32; i++){
			str[i] = '\0';
		}

		/* seleciona resposta para o cliente */
		switch(_command_code){
			case 0:
				snprintf(str, sizeof str, "%s%s", "Err!","\0"); break;
			case 1:
				snprintf(str, sizeof str, "%s%d%s%s", "Open#", _command_value, "!","\0"); break;
			case 2:
				snprintf(str, sizeof str, "%s%d%s", "Close#", _command_value, "!\0"); break;
			case 3:
				snprintf(str, sizeof str, "%s%d%s", "Level#", level, "!\0"); break;
			case 4: 
				snprintf(str, sizeof str, "%s", "Comm#OK!\0"); break;
			case 5:
				snprintf(str, sizeof str, "%s%d%s", "Max#", _command_value, "!\0"); break;
			case 6:
				snprintf(str, sizeof str, "%s%s", "Start#OK!", "\0"); break;
		}

		printf("sending answer to client %s\n", str);

		if(send(sock, str, received, 0) != received){
			Die("Failed to send bytes to client");
		}
		/* Check for more data */ 
		if((received = recv(sock, buffer, BUFFSIZE, 0)) < 0){
			Die("Failed to receive additional bytes from client");
		}	
	}
	close(sock);
}

int getCommandCode(){
	return _command_code;
}

int getCommandValue(){
	return _command_value;
}

/* TODO: create function to plot tank data */
void *TankGraph(){

}

void *Server(void *value){
	int *server_port = (int *) value;
	// server_port = atoi(server_port[1]);
	
	/* inicia o servidor */
	int serversock, clientsock;
	struct sockaddr_in echoserver, echoclient;

	// if (argc != 2) {
	// 	fprintf(stderr, "USAGE: echoserver <port>\n");
	// 	exit(1);
	// }
	/* Create the TCP socket */
	if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		Die("Failed to create socket");
	}
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
	echoserver.sin_port = htons(*server_port);       /* server port */
           
	/* Bind the server socket */
	if (bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
		Die("Failed to bind the server socket");
	}
	/* Listen on the server socket */
	if (listen(serversock, MAXPENDING) < 0) {
		Die("Failed to listen on server socket");
	}
	while(1){
		unsigned int clientlen = sizeof(echoclient);
		// printf("clientlen = %d \n", clientlen);
		/* Wait for client connection */
		if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
			Die("Failed to accept client connection");
		}
		fprintf(stdout, "Client connected: %s\n", inet_ntoa(echoclient.sin_addr));
		HandleClient(clientsock);
	}
}
