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

struct ServerInfo{
   char *Ip;
   char *Port;
   int Setpoint; 
};

int main(int argc, char *argv[]){

	char ch;

	struct ServerInfo *myServerInfo;
	myServerInfo = malloc(sizeof(struct ServerInfo));

	(*myServerInfo).Ip = argv[1];
	(*myServerInfo).Port = argv[2];
	(*myServerInfo).Setpoint = atoi(argv[3]);

	pthread_t tid;

	if (argc != 4){
		fprintf(stderr, "USAGE: TCPecho <server_ip> <word> <port>\n");
		exit(1);
	}

	clear();
	printf("Level Setpoint = %d\n", (*myServerInfo).Setpoint);

	// check communication
	myClient((*myServerInfo).Ip, (*myServerInfo).Port, "CommTest!");

	Command serverAnswer = getAnswer();

	if(serverAnswer.code == 4){ // comm ok! start process
		printf("Press ENTER key to Start!\n");
		scanf("%c",&ch);
		
		myClient((*myServerInfo).Ip, (*myServerInfo).Port, "Start!");

		// inicia o controlador
		pthread_create(&tid, NULL, Controller, (void *)&myServerInfo);

		while(1){
			printf("control running...\n");
			sleep(1);
    	}
	}
	else{ // comm not ok, kill process
		return 0;
	}

	/* Espera pelo setup do valor máximo do tanque */
	// while(serverAnswer.code != 5){
	// 	clear();
	// 	printf("==> Setup the Max Value first: SetMax#<value>!: ");
	// 	scanf("%s", argv[2]);
	// 	printf("\n");

	// 	myClient(argv[1], argv[2], argv[3]);
	// 	serverAnswer = getAnswer();
	// }
	// printf("Max Value set: %d", serverAnswer.value);

	// printf("Press ENTER key to Start!\n");    
	//here also if you press any other key will wait till pressing ENTER
	// scanf("%c",&ch);

	// myClient(argv[1], argv[2], "Start!");

	/* Espera pelo comando Start! */
	// while(serverAnswer.code != 6){
	// 	clear();
	// 	printf("==> Start the simulation: Start! ");
	// 	scanf("%s", argv[3]);
	// 	snprintf(argv[3], sizeof argv[3], "%s%s", argv[3], "\0");
	// 	printf("\n");

	// 	myClient(argv[1], argv[2], argv[3]);
	// 	serverAnswer = getAnswer();		
	// }

	// printf("Started!\n");

	/* Inicia o controle */
    //pthread_create(&tid, NULL, Control, (void *)&tid);

    
	
	// return 0;
}
