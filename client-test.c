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

#define T_CLIENTnsec 1000000000 //1000ms
#define T_CLIENTsec 1.0 //1000ms

#define BILLION 1000000000

//tamanho da janela que sera criada
#define SCREEN_W 640 
#define SCREEN_H 640

#define BPP 32

#define BUFFSIZE 255

#define MAXX 100
#define REF 80
#define Kp 9
#define Ki 0.03

typedef char *string;//rever

typedef struct ip_address {
char *ip;
char *port;
}Tip_address;

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

//variaveis globais
double level_p;
double in_angle;

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
//-------------------CLIENT_functions---------------------

void Die(char *mess) { perror(mess); exit(1); }


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
	
	data = datainit(SCREEN_W,SCREEN_H,200.0,110.0,level_p,in_angle,0.0);
	
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
			
			datadraw(data,t,level_p,in_angle,0.0);
			//fprintf(stderr, "level: %lf\n",level_p);
			//fprintf(stderr, "in_angle: %lf\n",in_angle);
			
			t+=T_GRAFsec;
			run=0;
		}
	}
}

void *client(void *addr){
	int sock;
	struct sockaddr_in echoserver;
	struct sockaddr_in echoclient;
	char buffer_r[BUFFSIZE];
	char buffer_s[BUFFSIZE];
	char value_str[BUFFSIZE];
	int value;
	unsigned int echolen, clientlen;
	int received = 0;
	
	int ok=0;
	double erro=0;
	double int_e=0;
	int delta=0;
	
	int run=1;

	struct timespec time_now, time_last;
	
	Tip_address *address = (Tip_address*) addr;
	
	/* Create the UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		Die("Failed to create socket");
	}
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(address->ip);  /* IP address */
	echoserver.sin_port = htons(atoi(address->port));       /* server port */	
	
	while(ok!=1){
		//---------------------------------------------send Setmax
		sprintf(buffer_s,"SetMax#%d!", (int) MAXX );
		echolen = strlen(buffer_s);
		if (sendto(sock, buffer_s, echolen, 0, (struct sockaddr *) &echoserver, sizeof(echoserver)) != echolen) 
			Die("Mismatch in number of sent bytes");
	
		/* Receive the word back from the server */
		clientlen = sizeof(echoclient);
		if ((received = recvfrom(sock, buffer_r, BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen)) < 0) 
			Die("Failed to receive message");

		/* Check that client and server are using same socket */
		if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) 
			Die("Received a packet from an unexpected server");

		//verifica resposta
		sprintf(buffer_s,"Max#%d!", (int) MAXX );
		if(strcmp(buffer_r, buffer_s) == 0)
			ok=1;
	}
	ok=0;	
	
	while(ok!=1){
		//---------------------------------------------send start
		echolen = strlen("Start!");
		if (sendto(sock, "Start!", echolen, 0, (struct sockaddr *) &echoserver, sizeof(echoserver)) != echolen) 
			Die("Mismatch in number of sent bytes");
	
		/* Receive the word back from the server */
		clientlen = sizeof(echoclient);
		if ((received = recvfrom(sock, buffer_r, BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen)) < 0) 
			Die("Failed to receive message");

		/* Check that client and server are using same socket */
		if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) 
			Die("Received a packet from an unexpected server");

		//verificar resposta
		if(strcmp(buffer_r,"Start#OK!") == 0)
			ok=1;
	}
	
	/* Run until cancelled */
	while (1) {
	
		//verifica tempo
		if(clock_gettime(CLOCK_MONOTONIC_RAW, &time_now))
			fprintf(stderr,"Clock_error");
	
		if(time_now.tv_sec == time_last.tv_sec){
			if((time_now.tv_nsec -time_last.tv_nsec) >=T_CLIENTnsec){
				run=1;
				time_last=time_now;
			}
		}
		else{
			if((time_now.tv_nsec-time_last.tv_nsec+BILLION) >=T_CLIENTnsec){
				run=1;
				time_last=time_now;
			}
		}
		if(run==1){
		
			//---------------------------------------------send Getlevel
			if(delta==0){
				echolen = strlen("GetLevel!");
				if (sendto(sock, "GetLevel!", echolen, 0, (struct sockaddr *) &echoserver, sizeof(echoserver)) != echolen) {
					Die("Mismatch in number of sent bytes");
				}
	
				/* Receive the word back from the server */
				clientlen = sizeof(echoclient);
				if ((received = recvfrom(sock, buffer_r, BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
					Die("Failed to receive message");
				}
				/* Check that client and server are using same socket */
				if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) {
					Die("Received a packet from an unexpected server");
				}
				
				// verificar resposta e armazenar level
				if(msg_type(buffer_r)==1){
					strcpy(buffer_s,strtok(buffer_r, "#"));
					strcpy(value_str,strtok(NULL, "!"));
									
					if(!strcmp(buffer_s,"Level") && (value = valid_num(value_str)) >= 0){
						level_p=value;
						//calculo do delta
						erro= REF - level_p;
						int_e+=erro*T_CLIENTsec;
						//fprintf(stderr, "INt_Erro: %d\n",int_e);
						//fprintf(stderr, "Erro: %d\n",erro);
						delta=(Kp*erro+Ki*int_e)-in_angle;
						//saturações
						if(delta + in_angle > 100.0)
							delta= 100-in_angle;
						if(delta + in_angle < 0.0)
							delta= -in_angle;
						//fprintf(stderr, "Delta: %d\n",delta);
					}else
						delta=0;
				}
			}			
			
			if(delta>0){
				//---------------------------------------------send Openvalve
				sprintf(buffer_s,"OpenValve#%d!", delta );
				echolen = strlen(buffer_s);
				if (sendto(sock, buffer_s, echolen, 0, (struct sockaddr *) &echoserver, sizeof(echoserver)) != echolen) {
					Die("Mismatch in number of sent bytes");
				}
	
				/* Receive the word back from the server */
				clientlen = sizeof(echoclient);
				if ((received = recvfrom(sock, buffer_r, BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
					Die("Failed to receive message");
				}
				/* Check that client and server are using same socket */
				if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) {
					Die("Received a packet from an unexpected server");
				}
				// verifica resposta
				sprintf(buffer_s,"Open#%d!", delta );
				if(strcmp(buffer_r, buffer_s) == 0)
					in_angle+=delta;
				
				delta=0;
			}
			
			if(delta<0){
				//---------------------------------------------send Closevalve
				sprintf(buffer_s,"CloseValve#%d!", abs(delta));
				echolen = strlen(buffer_s);
				if (sendto(sock, buffer_s, echolen, 0, (struct sockaddr *) &echoserver, sizeof(echoserver)) != echolen) {
					Die("Mismatch in number of sent bytes");
				}
	
				/* Receive the word back from the server */
				clientlen = sizeof(echoclient);
				if ((received = recvfrom(sock, buffer_r, BUFFSIZE, 0, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
					Die("Failed to receive message");
				}
				/* Check that client and server are using same socket */
				if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) {
					Die("Received a packet from an unexpected server");
				}
				// verifica resposta
				sprintf(buffer_s,"Close#%d!", abs(delta));
				if(strcmp(buffer_r, buffer_s) == 0)
					in_angle+=delta;
				
				delta=0;
			}
			run=0;	
		}	
	}	
	
	close(sock);
	exit(0);
}

//----------------------------------------------------
//---------------------MAIN---------------------------
int main(int argc, char *argv[]) {
	pthread_t thread_graf, thread_client;
	int  iret;
	Tip_address address; 
	
	char tecla='0';
	int out=0;
	
	//condições iniciais
	level_p =0.0;
	in_angle=50.0;
	
	
	if (argc != 3) {
		fprintf(stderr, "USAGE: %s <server_ip> <port>\n", argv[0]);
		exit(1);
	}

	address.ip=argv[1];
	address.port=argv[2];
	
	
	//criação das threads
	iret = pthread_create( &thread_graf, NULL, graf, NULL);
	if(iret){
		fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
		exit(EXIT_FAILURE);
	}
	sleep(1);
	iret = pthread_create( &thread_client, NULL, client, (void *) &address);
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