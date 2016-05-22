CC = gcc
CFLAGS = -D_REENTRANT -lpthread -Wall -pthread

all: parque

Parque: parque.c
	$(CC) parque.cc -o parque $(CFLAGS)

clean:
	rm parque
