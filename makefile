
	
all:	hconverter.o hconverter

clean:
	rm -f *.o hconverter

hconverter:	hconverter.o
	gcc -g -o hconverter *.o

hconverter.o:
	gcc -c -Wall -g src/*.c 
	
