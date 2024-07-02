#ifndef GREPPROCESS_H
#define GREPPROCESS_H

#include <regex.h>
#include <stddef.h>

typedef struct {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
  int h;
  int s;
  int f;
  int o;
  int x;
  char *pattern;
  size_t pattern_size;
  size_t lenPattern;
} options;

options grepOptions(int argc, char *argv[]);
void output(options flags, int argc, char *argv[]);
void outline(char *line, int n);
void printMatch(options flags, char *path, int lineCount, regex_t *reg,
                char *line);
void processFile(options flags, char *path, regex_t *reg, int argc);

#endif
