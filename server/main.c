#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>

#include <pthread.h> /* include para usar threads */

#include "decoder.h"
#include "data_types.h"
// #include "tank.h"

#define clear() printf("\033[H\033[J")

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Command clientCommands; 

double TankLevel = 40.0; // initial condition
double inValve = 50; // initial condition
double outValve = 0;
int outValveMax;

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
				snprintf(str, sizeof str, "%s%.3f%s", "Level#", TankLevel, "!\0"); break;
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

double delta = 0;

double getTime(){
	struct timespec tempo;
    clock_gettime(CLOCK_REALTIME, &tempo);
    return (double)(1000*tempo.tv_sec + tempo.tv_nsec/1000000.0);
}

double updateInput(double dT, double inAngle, Command command){
	if(command.code == 1){ // command to open input valve 
		delta += command.value;
	}
	else if(command.code == 2){ // command to close input valve
		delta -= command.value;
	}

	if(delta > 0){
		if(delta < 0.01*dT){
			inAngle += delta;
			delta = 0;
			if(inAngle > 100)inAngle = 100;
			
		}
		else{
			inAngle += 0.01*dT;
			delta -= 0.01*dT;
			if(inAngle > 100)inAngle = 100;
		}
	}
	else if(delta < 0){
		if(delta > -0.01*dT){
			inAngle += delta;
			delta = 0;
			if(inAngle < 0)inAngle = 0;
		}
		else{
			inAngle += -0.02*dT;
			delta += 0.01*dT;
			if(inAngle < 0)inAngle = 0;
		}
	}
	printf("[INFO] VALVE UPDATE , code %d, delta %f, angle%f\n", command.code, delta, inAngle);
	return inAngle;
}

double updateOutput(double T){
	if(T <= 0){
		return 50;
	}
	else if(T < 20000){
		return (50+T/400);
	}
	else if(T < 30000){
		return (100);
	}
	else if(T < 50000){
		return (100-(T-30000)/250);
	}
	else if(T < 70000){
		return (20+(T-50000)/1000);
	}
	else if(T < 100000){
		return (40+20*cos((T-70000)*2*M_PI/10000));
	}
	else{
		return 100;
	}
}

void *Tank(void* unused){
    
	Command tankCommands;

	double init_time, currente_time, pass_time, dT = 10, total_time = 0;
	double exec_time;
	double inFlux, outFlux;
	double delta = 0;

	// inFlux = 1*sin((M_PI/2)*(inValve/100));
	// outFlux = (outValve/100)*((TankLevel/1.25)+0.2)*sin((M_PI/2)*(outValve/100));

	// TankLevel = TankLevel + 0.00002*dT*(inFlux-outFlux);

	while(1){
		clear();
		init_time = getTime();
		// TODO: Adicionar uma 'MUTEX' com essas variáveis
		tankCommands.code = clientCommands.code;
		tankCommands.value = clientCommands.value;
		
		printf("[TANK INFO] Command code: %d, value: %d\n", clientCommands.code, clientCommands.value);
		printf("[INFO] Tank level = %.3f\n", TankLevel);

		if(tankCommands.code == 1){// open valve
			delta += tankCommands.value;
			if(delta > 100){
				delta = 100;
			}
		}
		else if(tankCommands.code == 2){// close valve
			delta -=tankCommands.value;
			if(delta < -100){
				delta = -100;
			}
		}

		inValve = updateInput(dT, inValve, tankCommands);
		outValve = updateOutput(total_time);

		inFlux = 1*sin((M_PI/2)*(inValve/100));
		outFlux = (outValveMax/100.0)*(TankLevel/125.0+0.2)*sin(M_PI/2*outValve/100);

		printf("[INFO] inFLux %f, outFlux %f\n", inFlux, outFlux);

		TankLevel = TankLevel + 0.00002*dT*(inFlux-outFlux);
		if(TankLevel < 0){
			TankLevel = 0;
		}
		else if(TankLevel > 100){
			TankLevel = 100;
		}
		
		
		

		tankCommands.code = 0;

		printf("[VALVES] in: %.3f, out: %0.3f\n", inValve, outValve);

		exec_time = getTime() - init_time;
		printf("[TIME] init: %.3f, current %.3f, dT: %-.3f, total time %.3f\n", init_time, currente_time, dT, total_time);
		

		// silumation time step
		if(exec_time < 10000.0){
			usleep(10000.0 - exec_time);
		}
		total_time += dT;
	}

	// usleep(10);
}

// "configure the server socket"
int main(int argc, char *argv[]) {
	pthread_t tid; 
	
	int server_port;

	outValve = atoi(argv[2]);
	outValveMax = outValve;
	server_port = atoi(argv[1]);

	/* Inicia thread do Server */
	pthread_create(&tid, NULL, Server, &server_port);
	
	clear();
	printf("Server ready! Max output set to: %.3f. Waiting for Start.\n", outValve);
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
	// TODO: Criar a thread do gráfico
	
	while(1){
		clear();
		printf("[INFO] Client command code: %d, value: %d; Tank level: %f\n", clientCommands.code, clientCommands.value, TankLevel);
        //clear();
        usleep(500000);
		
		// usleep(1000000);
		// printf("ainda nao estou preso num while :)\n");

		// pthread_create(&tid, NULL, Server(serversock, clientsock, echoclient), (void *)&tid);
	}
}
