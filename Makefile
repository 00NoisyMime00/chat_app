CC=gcc
CFLAGS=-lpthread
OBJ = sever.o client.o

all: server client

server: server.o
	$(CC) -o $@ $^ $(CFLAGS)

client: client.o
	$(CC) -o $@ client.o $(CFLAGS)