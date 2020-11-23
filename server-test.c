#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <SDL/SDL.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#define T_GRAFnsec 50000000 //50ms
#define T_GRAFsec 0.050 //50ms

#define T_PLANTnsec 10000000 //10ms
#define T_PLANTmsec 10.0 //10ms

#define T_SERVERnsec 10000000 //10ms

#define BILLION 1000000000

//tamanho da janela que sera criada
#define SCREEN_W 640 
#define SCREEN_H 640

#define BPP 32

#define BUFFSIZE 255//mudar dps

typedef char *string;//rever

typedef Uint32 PixelType;

typedef struct canvas {
  SDL_Surface *canvas;
  int Height; // canvas height
  int Width;  // canvas width
  int Xoffset; // X off set, in canvas pixels
  int Yoffset; // Y off set, in canvas pixels
  int Xext; // X extra width
  int Yext; // Y extra height
  double Xmax;
  double Ymax;
  double Xstep; // half a distance between X pixels in 'Xmax' scale

  PixelType *zpixel;

} Tcanvas;

typedef struct dataholder {
  Tcanvas *canvas;
  double   Tcurrent;
  double   Lcurrent;
  PixelType Lcolor;
  double   INcurrent;
  PixelType INcolor;
  double   OUTcurrent;
  PixelType OUTcolor;

} Tdataholder;

//variaveis a serem compartilhadas
double level_p;
double in_angle;
double out_angle;

int start;
double max;
pthread_mutex_t mutex_restart = PTHREAD_MUTEX_INITIALIZER;
int restart;
pthread_mutex_t mutex_delta = PTHREAD_MUTEX_INITIALIZER;
double delta;
//--------------------------------------------------------
//-------------------GRAF_functions-----------------------

inline void c_pixeldraw(Tcanvas *canvas, int x, int y, PixelType color)
{
  *( ((PixelType*)canvas->canvas->pixels) + ((-y+canvas->Yoffset) * canvas->canvas->w + x+ canvas->Xoffset)) = color;
}

inline void c_hlinedraw(Tcanvas *canvas, int xstep, int y, PixelType color)
{
  int offset =  (-y+canvas->Yoffset) * canvas->canvas->w;
  int x;

  for (x = 0; x< canvas->Width+canvas->Xoffset ; x+=xstep) {
        *( ((PixelType*)canvas->canvas->pixels) + (offset + x)) = color;
  }
}

inline void c_vlinedraw(Tcanvas *canvas, int x, int ystep, PixelType color)
{
  int offset = x+canvas->Xoffset;
  int y;
  int Ystep = ystep*canvas->canvas->w;

  for (y = 0; y< canvas->Height+canvas->Yext ; y+=ystep) {
    *( ((PixelType*)canvas->canvas->pixels) + (offset + y*canvas->canvas->w)) = color;
  }
}


inline void c_linedraw(Tcanvas *canvas, double x0, double y0, double x1, double y1, PixelType color) {
  double x;

  for (x=x0; x<=x1; x+=canvas->Xstep) {
    c_pixeldraw(canvas, (int)(x*canvas->Width/canvas->Xmax+0.5), (int)((double)canvas->Height/canvas->Ymax*(y1*(x1-x)+y1*(x-x0))/(x1-x0)+0.5),color);
  }
}


Tcanvas *c_open(int Width, int Height, double Xmax, double Ymax)
{
  int x,y;
  Tcanvas *canvas;
  canvas = malloc(sizeof(Tcanvas));

  canvas->Xoffset = 10;
  canvas->Yoffset = Height;

  canvas->Xext = 10;
  canvas->Yext = 10;



  canvas->Height = Height;
  canvas->Width  = Width; 
  canvas->Xmax   = Xmax;
  canvas->Ymax   = Ymax;

  canvas->Xstep  = Xmax/(double)Width/2;

  //  canvas->zpixel = (PixelType *)canvas->canvas->pixels +(Height-1)*canvas->canvas->w;

  SDL_Init(SDL_INIT_VIDEO); //SDL init
  canvas->canvas = SDL_SetVideoMode(canvas->Width+canvas->Xext, canvas->Height+canvas->Yext, BPP, SDL_SWSURFACE); 

  c_hlinedraw(canvas, 1, 0, (PixelType) SDL_MapRGB(canvas->canvas->format,  255, 255,  255));
  for (y=10;y<Ymax;y+=10) {
    c_hlinedraw(canvas, 3, y*Height/Ymax , (PixelType) SDL_MapRGB(canvas->canvas->format,  220, 220,  220));
  }
  c_vlinedraw(canvas, 0, 1, (PixelType) SDL_MapRGB(canvas->canvas->format,  255, 255,  255));
  for (x=10;x<Xmax;x+=10) {
    c_vlinedraw(canvas, x*Width/Xmax, 3, (PixelType) SDL_MapRGB(canvas->canvas->format,  220, 220,  220));
  }

  return canvas;
}



Tdataholder *datainit(int Width, int Height, double Xmax, double Ymax, double Lcurrent, double INcurrent, double OUTcurrent) {
  Tdataholder *data = malloc(sizeof(Tdataholder));


  data->canvas=c_open(Width, Height, Xmax, Ymax);
  data->Tcurrent=0;
  data->Lcurrent=Lcurrent;
  data->Lcolor= (PixelType) SDL_MapRGB(data->canvas->canvas->format,  255, 180,  0);
  data->INcurrent=INcurrent;
  data->INcolor=(PixelType) SDL_MapRGB(data->canvas->canvas->format,  180, 255,  0);
  data->OUTcurrent=OUTcurrent;
  data->OUTcolor=(PixelType) SDL_MapRGB(data->canvas->canvas->format,  0, 180,  255);


  return data;
}

void setdatacolors(Tdataholder *data, PixelType Lcolor, PixelType INcolor, PixelType OUTcolor) {
  data->Lcolor=Lcolor;
  data->INcolor=INcolor;
  data->OUTcolor=OUTcolor;
}

void datadraw(Tdataholder *data, double time, double level, double inangle, double outangle) {
  c_linedraw(data->canvas,data->Tcurrent,data->Lcurrent,time,level,data->Lcolor);
  c_linedraw(data->canvas,data->Tcurrent,data->INcurrent,time,inangle,data->INcolor);
  c_linedraw(data->canvas,data->Tcurrent,data->OUTcurrent,time,outangle,data->OUTcolor);
  data->Tcurrent = time;
  data->Lcurrent = level;
  data->INcurrent = inangle;
  data->OUTcurrent = outangle;

  SDL_Flip(data->canvas->canvas);
}

void quitevent() {
  SDL_Event event;

  while(SDL_PollEvent(&event)) { 
    if(event.type == SDL_QUIT) { 
      // close files, etc...

      SDL_Quit();
      exit(1); // this will terminate all threads !
    }
  }
}

//--------------------------------------------------------
//-------------------PLANT_functions----------------------

double out_calc(double time)
{
	if(time<=0)
		return 50;
	else if(time<20000)
		return 50+time/400;
	else if(time<30000)
		return 100;
	else if(time<50000)
		return 100-(time-30000)/250;
	else if(time<70000)
		return 20+(time-50000)/1000;
	else if(time<100000)
		return 40+20*cos((time-70000)*2*M_PI/10000);
	else
		return 100;
}

//--------------------------------------------------------
//-------------------SERVER_functions---------------------

void Die(char *mess) { perror(mess); exit(1); }

int option_select(char *msg, string *options, int n_op){
	int i;
	for(i=0 ; i < n_op; i++){
		//fprintf(stderr, "Testando: %s\n", options[i] );
		if(strcmp(msg, options[i]) == 0)
		return i;
	}
	return -1;
}
int is_num(char *msg){
	
	while(*msg != '\0'){
		if(*msg != '0' && *msg !='1' && *msg !='2' && *msg !='3' && *msg !='4' && *msg !='5' && *msg !='6' && *msg !='7' && *msg !='8' && *msg !='9' )
			return 0;
		msg++;
	}
	return 1;
}

int valid_num(char *msg){
	int num = -1;
	//char *num_s;
	//num_s = msg;

	if(is_num(msg) == 1){
		num = atoi(msg);
		if(num <= 100 && num >= 0 )
			return num;
	}
	return -1;
} 

int msg_type(char *msg){
	int state = 0;
	int type = -1;
	
	while(*msg != '\0'){
		switch (state){
		case 0:
			if(*msg == '#'){
				state = 4;
			} else if (*msg == '!'){
				state = 4;
			} else{
				state = 1;
			}
			break;
		case 1:
			if(*msg == '#'){
				state = 2;
			} else if (*msg == '!'){
				type = 2;
				state = 4;
			}
			break;				
		case 2:
			if(*msg == '!'){
				state = 4;
			} else{
				state = 3;
			}
			break;
		case 3:
			if(*msg == '!'){
				type = 1;
				state = 4;
			}
			break;
		default:
			type = -1;
			break;
		}
		msg++;
	}
	return type;
}

//-----------------------------------------------------------
//---------------------func_threads--------------------------
void *graf(){
	Tdataholder *data;
	int run=1;
	double t=0;

	struct timespec time_now, time_last;
	
	if(clock_gettime(CLOCK_MONOTONIC_RAW, &time_last))
		fprintf(stderr,"Clock_error");
	//tratar errors?
	
	data = datainit(SCREEN_W,SCREEN_H,200.0,110.0,level_p,in_angle,out_angle);
	
	while(1){
		
		//verifica tempo
		if(clock_gettime(CLOCK_MONOTONIC_RAW, &time_now))
			fprintf(stderr,"Clock_error");
	
		if(time_now.tv_sec == time_last.tv_sec){
			if((time_now.tv_nsec -time_last.tv_nsec) >=T_GRAFnsec){
				run=1;
				time_last=time_now;
			}
		}
		else{
			if((time_now.tv_nsec-time_last.tv_nsec+BILLION) >=T_GRAFnsec){
				run=1;
				time_last=time_now;
			}
		}
		
		//roda o grafico
		if(run==1){
			//fprintf(stderr,"yaw %f\n",t);
			
			datadraw(data,t,level_p*100.0,in_angle,out_angle);
			//fprintf(stderr, "level: %lf\n",level_p);
			//fprintf(stderr, "in_angle: %lf\n",in_angle);
			
			t+=T_GRAFsec;
			run=0;
		}
	}
}
void *plant(){
	int run=1;
	double t=0;
	double outflux, influx;
	
	struct timespec time_now, time_last;
	
	
	if(clock_gettime(CLOCK_MONOTONIC_RAW, &time_last))
		fprintf(stderr,"Clock_error");
	//else tratar errors
	

	while(1){
	
		//verifica tempo
		if(clock_gettime(CLOCK_MONOTONIC_RAW, &time_now))
			fprintf(stderr,"Clock_error");
	
		if(time_now.tv_sec == time_last.tv_sec){
			if((time_now.tv_nsec -time_last.tv_nsec) >=T_PLANTnsec){
				run=1;
				time_last=time_now;
			}
		}
		else{
			if((time_now.tv_nsec-time_last.tv_nsec+BILLION) >=T_PLANTnsec){
				run=1;
				time_last=time_now;
			}
		}
		
		if(run==1 &&  start==1){
			if(restart==1){
			in_angle=50.0;
			level_p=0.4;
			out_angle=50.0;
			t=0;
			
			pthread_mutex_lock( &mutex_restart );
			restart=0;//mutex!
			pthread_mutex_unlock( &mutex_restart  );	
			}
			
			influx=1*sin(M_PI/2.0*in_angle/100.0);
			outflux=(max/100.0)*(level_p/1.25+0.2)*sin(M_PI/2.0*out_angle/100.0);
			
			//fprintf(stderr,"level: %f\n",level_p);
			//fprintf(stderr,"in_ang: %f\n",in_angle);
			//fprintf(stderr,"out_ang: %f\n",out_angle);
	
			level_p=level_p +0.00002*T_PLANTmsec*(influx-outflux);//mutex?
	
			out_angle=out_calc(t);//mutex?
			
			if(delta > 0.0){
				if(delta < 0.01*T_PLANTmsec){
				in_angle=in_angle+delta;
				
				pthread_mutex_lock( &mutex_delta );
				delta=0.0;//mutex!
				pthread_mutex_unlock( &mutex_delta );	
				}
				else{
				in_angle=in_angle+0.01*T_PLANTmsec;
				
				pthread_mutex_lock( &mutex_delta );
				delta-=0.01*T_PLANTmsec;//mutex!
				pthread_mutex_unlock( &mutex_delta );

				}
			}
			else if(delta < 0.0){
				if(delta > -0.01*T_PLANTmsec){
					in_angle=in_angle+delta;	
					
					pthread_mutex_lock( &mutex_delta );
					delta=0.0;//mutex!
					pthread_mutex_unlock( &mutex_delta );
				}
				else{
					in_angle=in_angle-0.01*T_PLANTmsec;
					
					pthread_mutex_lock( &mutex_delta );
					delta +=0.01*T_PLANTmsec;//mutex!
					pthread_mutex_unlock( &mutex_delta );
				}
			}
			
			t+=T_PLANTmsec;
			run=0;
		}
	}
}

void *server(void *port){
	int sock;
	struct sockaddr_in echoserver;
	struct sockaddr_in echoclient;
	char buffer_r[BUFFSIZE];
	char buffer_s[BUFFSIZE];
	char option[BUFFSIZE];
	string options_1[3] = {"OpenValve", "CloseValve","SetMax"};// mudar
	string options_2[3] = {"GetLevel", "CommTest","Start"};
	char value_str[BUFFSIZE];
	char level_str[BUFFSIZE];
	unsigned int clientlen, serverlen;
	int received = 0;
	int msg_lenght = 0;
	int value = 0;
	
	
	int run=0;
	struct timespec time_now, time_last;
	
	if(clock_gettime(CLOCK_MONOTONIC_RAW, &time_last))
		fprintf(stderr,"Clock_error");
	//else tratar errors
	
	
	
	/* Create the UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		Die("Failed to create socket");
	}
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any IP address */
	echoserver.sin_port = htons(atoi((char *) port));       /* server port */

	/* Bind the socket */
	serverlen = sizeof(echoserver);
	if (bind(sock, (struct sockaddr *) &echoserver, serverlen) < 0) {
		Die("Failed to bind server socket");
	}
	
	/* Run until cancelled */
	while (1) {
	
		//verifica tempo
		if(clock_gettime(CLOCK_MONOTONIC_RAW, &time_now))
			fprintf(stderr,"Clock_error");
	
		if(time_now.tv_sec == time_last.tv_sec){
			if((time_now.tv_nsec -time_last.tv_nsec) >=T_SERVERnsec){
				run=1;
				time_last=time_now;
			}
		}
		else{
			if((time_now.tv_nsec-time_last.tv_nsec+BILLION) >=T_SERVERnsec){
				run=1;
				time_last=time_now;
			}
		}
		if(run==1){
			/* Receive a message from the client */
			clientlen = sizeof(echoclient);
			if ((received = recvfrom(sock, buffer_r, BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
				Die("Failed to receive message");
			}
			//fprintf(stderr, "Client connected: %s\n", inet_ntoa(echoclient.sin_addr));
			
			buffer_r[received]='\0';//tirar?
			//fprintf(stderr, "Recebido: %s\n",buffer_r);
			switch (msg_type(buffer_r)){
			case 1:
				strcpy(option,strtok(buffer_r, "#"));
				//fprintf(stderr, "Option: %s\n",option);
				strcpy(value_str,strtok(NULL, "!"));
				//fprintf(stderr, "value: %s\n",value_str);
				if((value = valid_num(value_str)) < 0)
					strcpy(buffer_s, "Err!");
				else{
					switch (option_select(option, options_1,3)){
					case 0:
						strcpy(buffer_s,"Open#");
						strcat(buffer_s,strcat(value_str,"!"));
						
						pthread_mutex_lock( &mutex_delta );
						delta=value;//mutex!
						pthread_mutex_unlock( &mutex_delta );
						break;
					case 1:
						strcpy(buffer_s,"Close#");
						strcat(buffer_s,strcat(value_str,"!"));
						
						pthread_mutex_lock( &mutex_delta );
						delta=-value;//mutex!
						pthread_mutex_unlock( &mutex_delta );
						break;
					case 2:
						strcpy(buffer_s,"Max#");
						strcat(buffer_s,strcat(value_str,"!"));
						max=value;
						break;
					default:
						strcpy(buffer_s, "Err!");
						break;
					}						
				}
				break;
			case 2:
				strcpy(option,strtok(buffer_r, "!"));
				switch (option_select(option, options_2, 3)){
				case 0:
					sprintf(level_str,"Level#%i!",(int) (level_p*100));
					strcpy(buffer_s, level_str);
					break;
				case 1:
					strcpy(buffer_s,"Comm#OK!");
					break;
				case 2:
					strcpy(buffer_s,"Start#OK!");
					if(start==0)
						start=1;
					else{
						pthread_mutex_lock( &mutex_restart );
						restart=1;//mutex!
						pthread_mutex_unlock( &mutex_restart  );
					}
					break;
				default:
					strcpy(buffer_s, "Err!");
					break;
				}			
				break;
			default:
				strcpy(buffer_s, "Err!");
				break;
		
			}
		
			msg_lenght = sizeof(buffer_s);	
			/* Send the message back to client */
			if (sendto(sock, buffer_s, msg_lenght, 0, (struct sockaddr *) &echoclient, sizeof(echoclient)) != msg_lenght)
				Die("Mismatch in number of echo'd bytes");
				
			run=0;
		}	
	}

}

//----------------------------------------------------
//---------------------MAIN---------------------------
int main( int argc, const char* argv[] ) {

	pthread_t thread_graf, thread_plant, thread_server;
	int  iret;
	
	char tecla='0';
	int out=0;
	
	//condições iniciais
	level_p =0.4;
	in_angle=50.0;
	out_angle=50.0;
	
	start=0;
	restart=0;
	max=100;
	delta=0;

	//verifica argumentos passados
	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <port>\n", argv[0]); 
		exit(1);
	}
	
	//criação das threads
	iret = pthread_create( &thread_graf, NULL, graf, NULL);
	if(iret){
		fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
		exit(EXIT_FAILURE);
	}
	sleep(1);
	iret = pthread_create( &thread_server, NULL, server, (void *) argv[1]);
	if(iret){
		fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
		exit(EXIT_FAILURE);
	}
	iret = pthread_create( &thread_plant, NULL, plant, NULL);
	if(iret){
		fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
		exit(EXIT_FAILURE);
	}
	
	while(out!=1){
	
	fprintf(stderr,"digite 's' para sair");
	scanf("%c",&tecla);
	if(tecla=='s')
		out=1;
		
	}
	fprintf(stderr,"bye");
	quitevent();
	return 0;
}