#ifndef CATPROCESS_H
#define CATPROCESS_H

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct shortFlags {
  int b, e, n, s, E, T, v, t, x;
} shortFlags;

shortFlags options(int argc, char **argv);
char nonPrintableOutput(shortFlags flags, unsigned char ch);
void outline(shortFlags flags, char *line, int n);
void processFile(FILE *file, shortFlags flags);
void readFile(int argc, char *argv[], shortFlags flags);

#endif