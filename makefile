CC = gcc
CFLAGS = -D_REENTRANT -lpthread -Wall -pthread

all: parque gerador

Parque: parque.c
	$(CC) parque.c -o parque $(CFLAGS)

Gerador: gerador.c
	$(CC) gerador.c -o gerador $(CFLAGS)
	
	
clean:
	rm parque
	rm gerador
