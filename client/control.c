/*

    TODO: resumo do código

*/

#include <stdio.h>
#include <unistd.h> 

#include "client.h"

struct ServerInfo{ 
   char *Ip;
   char *Port;
   int Setpoint; 
};

void *Controller(void *values){

	char *Ip;
	char *Port;
	int SetPoint;
	struct ServerInfo *controlServerInfo;
	
	controlServerInfo = (struct ServerInfo*) values;

	Ip = controlServerInfo->Ip;
	SetPoint = controlServerInfo->Setpoint;


	printf("Ip = %s; Port = %s; SetPoint = %d\n", Ip, Port, SetPoint);

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
}