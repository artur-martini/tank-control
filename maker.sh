rm Client
rm Server
gcc -o Server server/main.c server/decoder.c -lpthread -lm
gcc -o Client client/client.c client/decoder.c client/main.c -lpthread
