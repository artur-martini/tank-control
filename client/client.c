#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h> /* include para usar threads */

#define BUFFSIZE 32

#define clear() printf("\033[H\033[J")

/* Adicionar thread de controle */

/* Adicionar thread de exibição gráfica */

void Die(char *mess){
	perror(mess); exit(1); 
}

/* TODO: create function to plot control data */
void *ControlGraph(){

}

void myClient(char serverIP[10], char message[BUFFSIZE], char serverPort[10]){
	printf("my  client test: %s, %s, %s\n", serverIP, serverPort, message);

	int sock;
	struct sockaddr_in echoserver;
	char buffer[BUFFSIZE];
	unsigned int echolen;
	int received = 0;

	/* Create the TCP socket */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		Die("Failed to create socket");
	}
	
	// establish connection
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(serverIP);  /* IP address */
	echoserver.sin_port = htons(atoi(serverPort));       /* server port */
	/* Establish connection */
	if (connect(sock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
		Die("Failed to connect with server");
	}

	echolen = strlen(message);
	if (send(sock, message, echolen, 0) != echolen){
		Die("Mismatch in number of sent bytes");
	}
	/* Receive the word back from the server */
	fprintf(stdout, "Received: ");
	while (received < echolen){
		int bytes = 0;
		if ((bytes = recv(sock, buffer, BUFFSIZE-1, 0)) < 1){
			Die("Failed to receive bytes from server");
		}
		received += bytes;
		buffer[bytes] = '\0';        /* Assure null terminated string */
		fprintf(stdout, buffer);
	}

	// "wrapup"
	fprintf(stdout, "\n");
	close(sock);

}

