all: server

server: server.o
	gcc server.o -o server -g -lm -Wall

server.o: server.c
	gcc server.c -c -g -lm -Wall

clean:
	rm -f *.o server