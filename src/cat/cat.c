#define _GNU_SOURCE
#include "catProcess.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: cat [-belnstuv] [file ...]\n");
    return 1;
  }
  shortFlags flags = options(argc, argv);
  readFile(argc, argv, flags);
  return 0;
}
