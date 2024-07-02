CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c11

all: s21_cat

s21_cat: cat.o catProcess.o
	$(CC) $(CFLAGS) -o ./s21_cat cat.o catProcess.o

cat.o: cat.c
	$(CC) $(CFLAGS) -c cat.c

catProcess.o: catProcess.c
	$(CC) $(CFLAGS) -c catProcess.c

clean:
	rm -f *.o ./s21_cat a.out

rebuild: clean all

.PHONY: all clean rebuild