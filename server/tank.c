#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "server.h"
#include "data_types.h"

double delta = 0;

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
		}
		else{
			inAngle += 0.01*dT;
			delta -= 0.01*dT;
		}
	}
	else if(delta < 0){
		if(delta > -0.01*dT){
			inAngle += delta;
			delta = 0;
		}
		else{
			inAngle += -0.02*dT;
			delta += 0.01*dT;
		}
	}
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

/* Thread para simular a planta tanque */
void *Tank(void *value){
    int _command_code;
    int _command_value;
    int level = 20;

	// unsigned long init_time = clock();
	// int command_code = 0;
	// int command_value = 0;
	int valveAperture = 0;

	while(1){
		int *num = (int *) value;

		// TODO: Adicionar uma 'MUTEX' com essas variáveis
		_command_code = getCommandCode();
		_command_value = getCommandValue();

		printf("[TANK INFO] Command code: %d, value: %d\n", _command_code, _command_value);

		/* printa o comando recebido do controle no client */

		/* Ajuste do tempo de loop da planta, deve ser 10 ms */
		// unsigned long current_time = clock();
		// printf("%ld\n", current_time-init_time);
		// usleep(10000-(current_time-init_time));
		
		if(_command_code == 1){
			level ++;
		}
		else if(_command_code == 2){
			level --;
		}

		if(level >= 100){
			level = 100;
		}
		else if(level <= 0){
			level = 0;
		}
		
		printf("Tank, level:%d\n", level);
		usleep(1000000);
	}

	// usleep(10);
}
