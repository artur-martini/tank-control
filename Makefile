all: Server Client

Server: server/main.c
	gcc -o Server server/server.c server/main.c server/decoder.c -lpthread -lm

Client: client/main.c 
	gcc -o Client client/client.c client/decoder.c client/main.c -lpthread
	