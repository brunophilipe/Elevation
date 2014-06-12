all: main.o elevation.o
	mkdir -p ./build
	gcc main.o elevation.o -o ./build/elevation
	make clear

main.o: elevation.o
	gcc -c -O elevation.o main.c -o main.o

elevation.o:
	gcc -c -O elevation.c

clear:
	rm -rf *.o

clean:
	rm -rf ./build