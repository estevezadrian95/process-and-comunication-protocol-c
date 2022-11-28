CC = gcc
OFILES = server.o
CFLAGS = -g
#CFLAGS = -O
HEADERS = 
LIBS = -lpthread

server: $(OFILES)
	$(CC) $(CFLAGS) -o server $(OFILES) $(LIBS)

part1: server.c $(HEADERS)
	$(CC) $(CFLAGS) -c server.c