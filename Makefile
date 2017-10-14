CC = gcc

CFLAGS  = -g

all: main test1 test2
	$(CC) $(CFLAGS) main.c -o main
	$(CC) $(CFLAGS) test1.c -o test1
	$(CC) $(CFLAGS) test2.c -o test2
