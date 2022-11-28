CC = gcc
OFILES = client.o
CFLAGS = -g
#CFLAGS = -O
HEADERS = 
LIBS = -lpthread

client: $(OFILES)
	$(CC) $(CFLAGS) -o client $(OFILES) $(LIBS)

part1: client.c $(HEADERS)
	$(CC) $(CFLAGS) -c client.c