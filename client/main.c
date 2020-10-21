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

    pthread_create(&tid, NULL, Control, (void *)&tid);

    while(1){
        clear();
        sleep(1);
    }
	
	// if(strstr(buffer, "Start") != NULL){
	/* return command feedback */
	return 0;
}
