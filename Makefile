all: Server Client

Server: server/main.c
	gcc -o Server server/server.c server/main.c server/reader.c -pthread

Client: client/main.c 
	gcc -o Client client/client.c client/control.c client/main.c -pthread