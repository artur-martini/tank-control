#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> /* include para usar threads */

#include "server.h"
#include "tank.h"

#define clear() printf("\033[H\033[J")

/* comandos do client */
int command_code = 0;
int command_value = 0;

// "configure the server socket"
int main(int argc, char *argv[]) {
	pthread_t tid; 
	int server_port;

	int _tankMax = atoi(argv[2]);

	server_port = atoi(argv[1]);

	/* Inicia thread do Server */
	pthread_create(&tid, NULL, Server, &server_port);

	/* Espera pelo setup do Max Value */
	// while(getCommandCode() != 5){
	// 	_tankMax = getCommandValue();
	// 	clear();
	// 	printf("Waiting for Max Value setup...\n");
	// 	usleep(20000);
	// }

	/* Espera pelo start */
	while(getCommandCode() != 6){
		clear();
		printf("Ready! Waiting for start...\n");
		usleep(20000);
	}

	printf("Starting tank with Max flow output = %d \n", _tankMax);
	/* Inicia thread dp Tanque */
	pthread_create(&tid, NULL, Tank, &command_code);
	/* Inicia thread gráficos */
	// TODO: Janela gráfica
	
	while(1){

        clear();
        usleep(10000);
		
		// usleep(1000000);
		// printf("ainda nao estou preso num while :)\n");

		// pthread_create(&tid, NULL, Server(serversock, clientsock, echoclient), (void *)&tid);
	}
}
