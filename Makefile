all: main
	g++ -o main main.o

main:
	g++ -c main.cpp

clean:
	rm -rf main *.o *~
