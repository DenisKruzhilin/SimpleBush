#define _GNU_SOURCE
#include "catProcess.h"
// Функция для разбора командных опций и установки соответствующих флагов
shortFlags options(int argc, char **argv) {
  shortFlags flags = {0};

  // Определяем длинные опции, соответствующие коротким опциям
  struct option long_options[] = {{"number-nonblank", no_argument, NULL, 'b'},
                                  {"number", no_argument, NULL, 'n'},
                                  {"squeeze-blank", no_argument, NULL, 's'},
                                  {NULL, 0, NULL, 0}};

  int opt;
  opterr = 0;  // Подавляем автоматические сообщения об ошибках от getopt
  while ((opt = getopt_long(argc, argv, "+bnEestT", long_options, NULL)) !=
         -1) {
    switch (opt) {
      case 'b':  // Нумеровать только непустые строки вывода
        flags.b = 1;
        break;
      case 'e':  // Отображать непечатаемые символы и метки конца строки
        flags.E = 1;
        flags.v = 1;
        break;
      case 'E':  // Отображать метки конца строки
        flags.E = 1;
        break;
      case 'n':  // Нумеровать все строки вывода
        flags.n = 1;
        break;
      case 's':  // Уменьшать несколько смежных пустых строк до одной
        flags.s = 1;
        break;
      case 't':  // Отображать непечатаемые символы и символы табуляции
        flags.T = 1;
        flags.v = 1;
        break;
      case 'T':  // Отображать символы табуляции как ^I
        flags.T = 1;
        break;
      case '?':  // Неопознанная опция
        fprintf(stderr, "cat: illegal option -- %c\n", optopt);
        fprintf(stderr, "usage: cat [-belnstuv] [file ...]\n");
        flags.x = 1;
    }
  }

  if (flags.x) {
    exit(EXIT_FAILURE);
  }

  return flags;  // Возвращаем установленные флаги
}
// Функция для обработки непечатаемых символов в зависимости от флагов
char nonPrintableOutput(shortFlags flags, unsigned char ch) {
  if (ch == '\n' || ch == '\t') {
    if (flags.T && ch == '\t') {
      printf("^I");
    } else {
      return ch;  // Возвращаем символ, если это новая строка или табуляция без
                  // преобразования
    }
  }

  // Обрабатываем управляющие символы и символ DEL
  if (ch <= 31 || ch == 127) {
    putchar('^');
    if (ch == 127) {
      ch = '?';
    } else {
      ch += 64;
    }
  } else if (ch >= 128 && ch <= 159) {
    printf("M-");
    putchar('^');
    ch -= 64;
  }
  return ch;
}

// Функция для обработки одной строки в зависимости от флагов
void outline(shortFlags flags, char *line, int n) {
  for (int i = 0; i < n; i++) {
    if (flags.s && line[i] == '\n') {
      while (i + 1 < n && line[i + 1] == '\n') {
        i++;
        printf("\n");
      }
    }
    if (flags.T && line[i] == '\t') {
      printf("^I");
    } else {
      if ((flags.E && flags.b) || (flags.e && flags.b)) {
        if (i == 0 && line[i] == '\n') {
          printf("%8c\t", '$');
        } else if (line[i] == '\n' && i == n - 1) {
          putchar('$');
        }
      } else if (flags.E && line[i] == '\n' && i == n - 1) {
        putchar('$');
      }
      if (flags.v) {
        line[i] = nonPrintableOutput(flags, line[i]);
      }
      putchar(line[i]);
    }
  }
}

// Функция для обработки файла в зависимости от флагов
void processFile(FILE *file, shortFlags flags) {
  char *line = NULL;
  size_t memCount = 0;
  int readChars = 0;
  int lineCount = 1;
  int count = 0;

  readChars = getline(&line, &memCount, file);

  while (readChars != -1) {
    if (line[0] == '\n') {
      count++;
    } else {
      count = 0;
    }

    if (flags.s && count > 1) {
      readChars = getline(&line, &memCount, file);
      continue;
    } else {
      if (flags.n || flags.b) {
        if (flags.b && line[0] != '\n') {
          printf("%6d\t", lineCount);
          lineCount++;

        } else if (flags.n) {
          printf("%6d\t", lineCount);
          lineCount++;
        }
      }
      outline(flags, line, readChars);
      readChars = getline(&line, &memCount, file);
    }
  }
  free(line);
}

// Функция для чтения и обработки всех файлов, переданных через командную строку
void readFile(int argc, char *argv[], shortFlags flags) {
  for (char **filename = &argv[optind], **end = &argv[argc]; filename != end;
       ++filename) {
    if (**filename == '-' || strcmp(*filename, "--") == 0) {
      continue;
    }
    FILE *file = fopen(*filename, "r");
    if (file == NULL) {
      fprintf(stderr, "cat: %s: No such file or directory\n", *filename);
      continue;
    }
    optind = 1;
    processFile(file, flags);
    fclose(file);
  }
}
