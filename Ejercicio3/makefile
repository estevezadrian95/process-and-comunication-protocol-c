ejecutables = productor-asis productor-pagos consumer launcher
libs = leeLinea.o lista.o sharedmem.o

all: $(ejecutables)
launcher: main-launcher.c 
	gcc -pthread -o Ejercicio3.bin -g main-launcher.c sharedmem.o

productor-asis: main-asis.o 
	gcc -pthread -o productor-asis.bin main-asis.o $(libs)

productor-pagos: main-pagos.o
	gcc -pthread -o productor-pagos.bin main-pagos.o $(libs)

consumer: main-consumer.o
	gcc -pthread -o consumer.bin main-consumer.o $(libs)

main-asis.o: main-asis.c libs
	gcc -c -g main-asis.c

main-pagos.o: main-pagos.c libs
	gcc -c -g main-pagos.c

main-consumer.o: main-consumer.c libs
	gcc -c -g main-consumer.c

libs: leeLinea.o lista.o common.h sharedmem

sharedmem: sharedmem.c sharedmem.h common.h
	gcc -c -g sharedmem.c

leeLinea.o: leeLinea.c leeLinea.h
	gcc -c -g leeLinea.c

lista.o: lista.c lista.h
	gcc -c -g lista.c

clean:
	rm *.o