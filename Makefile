all: Server Client

Server: server/main.c
	gcc -o Server server/server.c server/tank.c server/main.c server/decoder.c -pthread

Client: client/main.c 
	gcc -o Client client/client.c client/decoder.c client/control.c client/main.c -pthread