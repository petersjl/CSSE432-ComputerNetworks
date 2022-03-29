all: server client

server: server.o
	gcc server.o -o server -g -lm -Wall

server.o: server.c
	gcc server.c -c -g -lm -Wall

client: client.o
	gcc client.o -o client -g -lm -Wall

client.o: client.c
	gcc client.c -c -g -lm -Wall

clean:
	rm -f *.o server client