#include <stdio.h>
#include <unistd.h>

#include "server.h"


/* Thread para simular a planta tanque */
void *Tank(void *value){
    int _command_code;
    int _command_value;
    int level;

	// unsigned long init_time = clock();
	// int command_code = 0;
	// int command_value = 0;
	int valveAperture = 0;

	while(1){
		int *num = (int *) value;

		// TODO: Adicionar uma 'MUTEX' com essas variáveis
		_command_code = getCommandCode();
		_command_value = getCommandValue();

		printf("command code: %d, value: %d\n", _command_code, _command_value);

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
