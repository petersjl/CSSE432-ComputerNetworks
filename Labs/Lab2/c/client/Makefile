all: client

client: client.o
	gcc client.o -o client -g -lm -Wall

client.o: client.c
	gcc client.c -c -g -lm -Wall

clean:
	rm -f *.o client