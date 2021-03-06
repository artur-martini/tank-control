#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h> /* include para usar threads */

#include "client.h"
// #include "control.h"

#define clear() printf("\033[H\033[J")
#define SLEEP_CONTROLLER 50000// em us

struct ServerInfo{
   char* Ip;
   char* Port;
   int Setpoint; 
};

int TankLevel = 50;

void *Controller(void *input){
	
	Command serverAnswer;

	char message[32];
	int K = 5;
	int valveValueRef = 0;

	struct ServerInfo myServerInfo;

	myServerInfo.Ip = ((struct ServerInfo*)input)->Ip;
	myServerInfo.Port = ((struct ServerInfo*)input)->Port;
	myServerInfo.Setpoint = ((struct ServerInfo*)input)->Setpoint;

	printf("[INFO] Controller running\n");

	while(1){
		clear();
		printf("[INFO] Settings: %s, %s, %d\n", myServerInfo.Ip, myServerInfo.Port, myServerInfo.Setpoint);
		
		// get level
		serverAnswer = myClient(myServerInfo.Ip, myServerInfo.Port, "GetLevel!");
		if(serverAnswer.code == 3){
			TankLevel = serverAnswer.value;
		}
		printf("[INFO] Tank Level = %d\n", TankLevel);


		//TODO: tirar esse teste e implementar um controlador PID
		// teste com controle bang bang
		// snprintf(str, sizeof str, "%s%d%s%s", "Open#", clientCommands.value, "!","\0"); break;
		if(TankLevel < myServerInfo.Setpoint){
			valveValueRef = K*(myServerInfo.Setpoint - TankLevel); // sinal de controle

			if(valveValueRef > 100){
				valveValueRef = 100;
			}

			snprintf(message, sizeof message, "%s%d%s", "OpenValve#", valveValueRef, "!\0");
			serverAnswer = myClient(myServerInfo.Ip, myServerInfo.Port, message);
			printf("[INFO] Client message: %s\n", message);
		}
		else if(TankLevel > myServerInfo.Setpoint){
			valveValueRef = abs(K*(myServerInfo.Setpoint - TankLevel)); // sinal de controle
			
			if(valveValueRef > 100){
				valveValueRef = 100;
			}

			snprintf(message, sizeof message, "%s%d%s", "CloseValve#", valveValueRef, "!\0");
			serverAnswer = myClient(myServerInfo.Ip, myServerInfo.Port, message);
			printf("[INFO] Client message: %s\n", message);
		}

		printf("[INFO] Server answer: code=%d value=%d\n", serverAnswer.code, serverAnswer.value);

		usleep(500000);
	}

}

// Referência de comandos:
//
//  1 - OpenValve, sintaxe: OpenValve#<value>!
// 	2 - CloseVale, sintaxe: CloseValve#<value>!
// 	3 - GetLevel, sintaxe: GetLevel!
// 	4 - CommTest, sintaxe: CommTest!
// 	5 - SetMax, sintaxe: SetMax#<value>!
// 	6 - Start, sintaxe: Start!	

int main(int argc, char *argv[]){
	clear();

	char ch;

	struct ServerInfo *myServerInfo = (struct ServerInfo *)malloc(sizeof(struct ServerInfo));

	myServerInfo->Ip = argv[1];
	myServerInfo->Port = argv[2];
	myServerInfo->Setpoint = atoi(argv[3]);

	// max setpoint = 100
	if(myServerInfo->Setpoint > 100){
		myServerInfo->Setpoint = 100;
	}
	else if(myServerInfo->Setpoint < 0){
		myServerInfo->Setpoint = 0;
	}

	pthread_t tid;

	// (*myServerInfo).Ip = argv[1];
	// (*myServerInfo).Port = argv[2];
	// (*myServerInfo).Setpoint = atoi(argv[3]);

	printf("DEBUG: server info %s %s %d\n", myServerInfo->Ip, 
		myServerInfo->Port, myServerInfo->Setpoint);

	if (argc != 4){
		fprintf(stderr, "USAGE: TCPecho <server_ip> <word> <port>\n");
		exit(1);
	}

	printf("Level Setpoint = %d\n", (*myServerInfo).Setpoint);

	// check communication
	myClient((*myServerInfo).Ip, (*myServerInfo).Port, "CommTest!");

	Command serverAnswer = getAnswer();

	printf("server answer = %d %d\n", serverAnswer.code, serverAnswer.value);

	if(serverAnswer.code == 4){ // comm ok! start process
		printf("Press ENTER key to Start!\n");
		scanf("%c",&ch);
		
		myClient((*myServerInfo).Ip, (*myServerInfo).Port, "Start!");

		printf("started!");
		// inicia o controlador
		pthread_create(&tid, NULL, Controller, (void *)myServerInfo);
		pthread_join(tid, NULL);

		// TODO: criar a thread do gráfico
		
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

    
	
	return 0;
}
