CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c11

all: s21_grep

s21_grep: grep.o grepProcess.o
	$(CC) $(CFLAGS) -o ./s21_grep grep.o grepProcess.o

grep.o: grep.c
	$(CC) $(CFLAGS) -c grep.c


grepProcess.o: grepProcess.c
	$(CC) $(CFLAGS) -c grepProcess.c


clean:
	rm -f *.o ./s21_grep a.out

rebuild: clean all

.PHONY: all clean rebuild
