/*

    TODO: resumo do código

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#include "client.h"

void *Controller(void *value){

	// char *Ip;
	// char *Port;
	int *setpoint = (int *) value;
	// struct ServerInfo *controlServerInfo;
	
	// controlServerInfo = (struct ServerInfo*)values;

	// Ip = controlServerInfo->Ip;
	// SetPoint = controlServerInfo->Setpoint;
	printf("im here!\n");

	printf("SetPoint = %d\n", *setpoint);
	

	// printf("debuging this shit %d\n", ((struct ServerInfo*)values)->Setpoint);

	// printf("control started!\n");
	// int P = 0;
	// int I = 0;
	// int D = 0;

	// Command serverAnswer;

	// char serverIP[10];
	// char message[BUFFSIZE];
	// char serverPort[10];

	while(1){

		// myClient("127.0.0.1", "OpenValve#10!", "5912");
		// serverAnswer = getAnswer();

        // printf("Control report: code=%d, value=%d\n", serverAnswer.code, serverAnswer.value);
        sleep(1);
    
    /* read level */
    
	/* read reference */

	/* get control value */

	/* send control signal */	
	}
	return NULL;
}