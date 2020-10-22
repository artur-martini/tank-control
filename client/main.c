#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h> /* include para usar threads */

#include "client.h"
#include "control.h"

#define clear() printf("\033[H\033[J")

int main(int argc, char *argv[]){

	if (argc != 4){
		fprintf(stderr, "USAGE: TCPecho <server_ip> <word> <port>\n");
		exit(1);
	}

    pthread_t tid;

	myClient(argv[1], argv[2], argv[3]);

	Command serverAnswer = getAnswer();

	/* Espera pelo setup do valor máximo do tanque */
	while(serverAnswer.code != 5){
		clear();
		printf("==> Setup the Max Value first: SetMax#<value>!: ");
		scanf("%s", argv[2]);
		printf("\n");

		myClient(argv[1], argv[2], argv[3]);
		serverAnswer = getAnswer();
	}
	printf("Max Value set: %d", serverAnswer.value);

	/* Espera pelo comando Start! */
	while(serverAnswer.code != 6){
		clear();
		printf("==> Start the simulation: Start! ");
		scanf("%s", argv[2]);
		snprintf(argv[2], sizeof argv[2], "%s%s", argv[2], "\0");
		printf("\n");

		myClient(argv[1], argv[2], argv[3]);
		serverAnswer = getAnswer();		
	}

	printf("started!\n");

	/* Inicia o controle */
    //pthread_create(&tid, NULL, Control, (void *)&tid);

    while(1){
        printf("ok...\n");
        sleep(1);
    }
	
	return 0;
}
