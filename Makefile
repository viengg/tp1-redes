all:
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o cliente
	gcc -Wall server.c common.o -o servidor

clean:
	rm common.o cliente servidor 
