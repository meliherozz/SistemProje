hepsi: derle calistir

derle:
	g++ -I ./include -o ./lib/shell.o -c ./src/shell.c
	g++ -I ./include -o ./lib/io.o -c ./src/io.c
	g++ -I ./include -o ./lib/background.o -c ./src/background.c
	g++ -I ./include -o ./lib/pipe.o -c ./src/pipe.c
	g++ -I ./include -o ./bin/main ./lib/pipe.o ./lib/io.o ./lib/background.o ./lib/shell.o ./src/main.c

calistir:
	./bin/main

