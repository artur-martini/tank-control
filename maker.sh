rm Client
rm Server
gcc -o Server server/server.c server/tank.c server/main.c server/decoder.c -pthread -lm
gcc -o Client client/client.c client/decoder.c client/control.c client/main.c -pthread