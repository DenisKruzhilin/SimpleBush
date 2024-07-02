#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "grepProcess.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr,
            "Usage: %s [-e pattern] [-i] [-v] [-c] [-l] [-n] [-h] [-s] [-f "
            "file] [-o] [pattern] [file...]\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  options flags = grepOptions(argc, argv);
  output(flags, argc, argv);

  return EXIT_SUCCESS;
}
