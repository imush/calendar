
	
all:	converter.o converter

clean:
	rm *.o converter

converter:	converter.o
	gcc -g -o converter *.o

converter.o:
	gcc -c -Wall -g src/*.c 