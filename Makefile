all: server client

server: server.c
	gcc -o server server.c -lpthread

client: client.c
	gcc -o client client.c

clean:
	rm -f server client
