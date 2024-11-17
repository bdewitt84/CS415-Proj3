CC = gcc

all: part1


part1: part1.o string_parser.o
	$(CC) -pthread -lpthread $^ -o $@

string_parser.o: string_parser.c string_parser.h
	$(CC) -g -c string_parser.c


clean:
	rm -f core *.o part1
