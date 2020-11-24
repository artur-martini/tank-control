#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h> /* include para usar threads */

#include "decoder.h"
#include "data_types.h"
// #include "tank.h"

#define clear() printf("\033[H\033[J")

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Command clientCommands;

int TankLevel = 50;
int FLAG_START = 0; // flag to start the tank simulation

#define MAXPENDING 5 /* max connection requests */
#define BUFFSIZE 32

/* define o tamanho máximo da mensagem recebida */
#define MESSAGE_SIZE 32

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
	
	// printf("buffer %s \n", buffer);
	Command server_command;

	server_command.code = 0;
	server_command.value = 0;

	server_command = getClientCommand(buffer);

	// update client commands
	clientCommands.code = server_command.code;
	clientCommands.value = server_command.value;

	if(clientCommands.code == 6){ // this is bad
		FLAG_START = 1;
	}
	// printf("==> Update code: %d, value: %d\n", _command_code, _command_value);

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
		switch(clientCommands.code){
			case 0:
				snprintf(str, sizeof str, "%s%s", "Err!","\0"); break;
			case 1:
				snprintf(str, sizeof str, "%s%d%s%s", "Open#", clientCommands.value, "!","\0"); break;
			case 2:
				snprintf(str, sizeof str, "%s%d%s", "Close#", clientCommands.value, "!\0"); break;
			case 3:
				snprintf(str, sizeof str, "%s%d%s", "Level#", TankLevel, "!\0"); break;
			case 4: 
				snprintf(str, sizeof str, "%s", "Comm#OK!\0"); break;
			case 5:
				snprintf(str, sizeof str, "%s%d%s", "Max#", clientCommands.value, "!\0"); break;
			case 6:
				snprintf(str, sizeof str, "%s%s", "Start#OK!", "\0"); break;
		}

		// printf("sending answer to client %s\n", str);

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
		// fprintf(stdout, "Client connected: %s\n", inet_ntoa(echoclient.sin_addr));
		HandleClient(clientsock);
	}
}


/* Thread para simular a planta tanque */
void *Tank(void* unused){
    
	Command tankCommands;

	// unsigned long init_time = clock();
	// int command_code = 0;
	// int command_value = 0;
	int valveAperture = 0;

	while(1){
		

		// TODO: Adicionar uma 'MUTEX' com essas variáveis
		

		tankCommands.code = clientCommands.code;
		tankCommands.value = clientCommands.value;
		
		

		printf("[TANK INFO] Command code: %d, value: %d\n", clientCommands.code, clientCommands.value);

		/* printa o comando recebido do controle no client */

		/* Ajuste do tempo de loop da planta, deve ser 10 ms */
		// unsigned long current_time = clock();
		// printf("%ld\n", current_time-init_time);
		// usleep(10000-(current_time-init_time));
		
		if(clientCommands.code == 1){
			TankLevel ++;
		}
		else if(clientCommands.code == 2){
			TankLevel --;
		}

		if(TankLevel >= 100){
			TankLevel = 100;
		}
		else if(TankLevel <= 0){
			TankLevel = 0;
		}
		
		printf("Tank, level:%d\n", TankLevel);
		usleep(100000);
	}

	// usleep(10);
}

// "configure the server socket"
int main(int argc, char *argv[]) {
	pthread_t tid; 
	
	int server_port;

	int _tankMaxOuput = atoi(argv[2]);

	server_port = atoi(argv[1]);

	/* Inicia thread do Server */
	pthread_create(&tid, NULL, Server, &server_port);
	
	clear();
	printf("Server ready! Max output set to: %d. Waiting for Start.\n", _tankMaxOuput);
	/* Espera pelo start */
	while(FLAG_START == 0){
		usleep(50000);
	}
	clear();
	printf("[INFO] Starting tank!\n");
	sleep(2);

	/* Inicia thread dp Tanque */
	pthread_create(&tid, NULL, Tank, NULL);

	/* Inicia thread gráficos */
	// TODO: Janela gráfica
	
	while(1){
		clear();
		printf("[INFO] Client command code: %d, value: %d; Tank level: %d\n", clientCommands.code, clientCommands.value, TankLevel);
        //clear();
        usleep(500000);
		
		// usleep(1000000);
		// printf("ainda nao estou preso num while :)\n");

		// pthread_create(&tid, NULL, Server(serversock, clientsock, echoclient), (void *)&tid);
	}
}
