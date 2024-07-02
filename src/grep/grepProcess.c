#define _GNU_SOURCE
#include "grepProcess.h"

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Функция для добавления нового паттерна к текущим паттернам
void patternAdd(options *flags, char *pattern) {
  // Определяем новый размер строки паттерна
  size_t new_size =
      flags->lenPattern + strlen(pattern) + 3;  // 3 для скобок и '|'

  // Расширяем память для нового паттерна
  char *new_pattern = realloc(flags->pattern, new_size);
  if (new_pattern == NULL) {
    perror("Unable to reallocate memory");
    free(flags->pattern);
    exit(EXIT_FAILURE);
  }

  // Обновляем указатель на паттерн
  flags->pattern = new_pattern;

  // Если это первый паттерн, добавляем его с окружением в скобки
  if (flags->lenPattern == 0) {
    flags->lenPattern +=
        sprintf(flags->pattern + flags->lenPattern, "(%s)", pattern);
  } else {
    // Если уже есть паттерны, добавляем новый с разделителем '|'
    flags->lenPattern +=
        sprintf(flags->pattern + flags->lenPattern, "|(%s)", pattern);
  }
}

// Функция для добавления паттернов из файла
void addRegFromFile(options *flags, char *filepath) {
  // Открываем файл для чтения
  FILE *file = fopen(filepath, "r");
  if (file == NULL) {
    // Если не удалось открыть файл и флаг 's' не установлен, выводим ошибку
    if (!flags->s) {
      perror(filepath);
    }
    free(flags->pattern);  // Освобождаем память при ошибке
    exit(EXIT_FAILURE);
  }

  char *line = NULL;
  size_t memlen = 0;
  ssize_t read;

  // Читаем файл построчно
  while ((read = getline(&line, &memlen, file)) != -1) {
    // Убираем символ новой строки, если он присутствует
    if (line[read - 1] == '\n') {
      line[read - 1] = '\0';
    }
    // Добавляем прочитанную строку как новый паттерн
    patternAdd(flags, line);
  }

  // Освобождаем память для строки и закрываем файл
  free(line);
  fclose(file);
}

// Функция для обработки опций командной строки
options grepOptions(int argc, char *argv[]) {
  // Инициализация структуры флагов и выделение памяти для паттерна
  options flags = {0};
  flags.pattern = malloc(1024);  // Изначальное выделение памяти для паттерна
  if (flags.pattern == NULL) {
    perror("Unable to allocate memory");
    exit(EXIT_FAILURE);
  }
  flags.lenPattern = 0;

  int opt;
  // Обработка опций командной строки
  while ((opt = getopt(argc, argv, "+e:ivclnhsf:o")) != -1) {
    switch (opt) {
      case 'e':  // Опция для добавления паттерна
        flags.e = 1;
        patternAdd(&flags, optarg);
        break;
      case 'i':  // Опция для нечувствительности к регистру
        flags.i = REG_ICASE;
        break;
      case 'v':  // Опция для инверсии совпадений
        flags.v = 1;
        break;
      case 'c':  // Опция для подсчета количества совпадений
        flags.c = 1;
        break;
      case 'l':  // Опция для вывода только имен файлов с совпадениями
        flags.l = 1;
        break;
      case 'n':  // Опция для вывода номеров строк с совпадениями
        flags.n = 1;
        break;
      case 'h':  // Опция для подавления вывода имен файлов
        flags.h = 1;
        break;
      case 's':  // Опция для подавления сообщений об ошибках файлов
        flags.s = 1;
        break;
      case 'f':  // Опция для чтения паттернов из файла
        flags.f = 1;
        addRegFromFile(&flags, optarg);
        break;
      case 'o':  // Опция для вывода только совпадающих частей строк
        flags.o = 1;
        break;
      default:
        // Вывод сообщения об использовании программы при неверной опции
        flags.x = 1;
        fprintf(stderr,
                "Usage: %s [-e pattern] [-i] [-v] [-c] [-l] [-n] [-h] [-s] [-f "
                "file] [-o] [pattern] [file...]\n",
                argv[0]);
    }
  }

  // Если паттерн не был задан, добавляем его из оставшихся аргументов
  if (flags.lenPattern == 0 && optind < argc) {
    patternAdd(&flags, argv[optind]);
    optind++;
  }

  // Если обработан только один файл, устанавливаем флаг 'h'
  if (argc - optind == 1) {
    flags.h = 1;
  }

  if (flags.x == 1) {
    free(flags.pattern);  // Освобождение памяти при ошибке
    exit(EXIT_FAILURE);
  }

  return flags;
}

// Функция для вывода строки
void outline(char *line, int n) {
  // Выводим строку до указанного размера
  for (int i = 0; i < n; i++) {
    putchar(line[i]);
  }

  // Если последний символ не является переводом строки, добавляем его
  if (line[n - 1] != '\n') {
    putchar('\n');
  }
}

// Функция для вывода совпадений в строке
void printMatch(options flags, char *path, int lineCount, regex_t *reg,
                char *line) {
  regmatch_t match;
  int displacement = 0;

  // Ищем совпадения в строке
  while (regexec(reg, line + displacement, 1, &match, 0) == 0) {
    // Выводим имя файла, если флаг 'h' не установлен и обрабатывается более
    // одного файла
    if (!flags.h) {
      printf("%s:", path);
    }

    // Выводим номер строки, если флаг 'n' установлен
    if (flags.n) {
      printf("%d:", lineCount);
    }

    // Выводим совпавший паттерн
    for (int i = match.rm_so; i < match.rm_eo; i++) {
      putchar(line[displacement + i]);
    }
    putchar('\n');

    // Смещаемся на конец совпадения, чтобы искать следующее совпадение
    displacement += match.rm_eo;
  }
}

// Функция для обработки одного файла
void processFile(options flags, char *path, regex_t *reg, int argc) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    // Если не удалось открыть файл и флаг 's' не установлен, выводим ошибку
    if (!flags.s) {
      perror(path);
    }
    return;  // Просто вернуться, чтобы обработать следующий файл
  }

  char *line = NULL;
  size_t memlen = 0;
  ssize_t stringSize;
  int lineCount = 1;
  int coincidence = 0;

  // Для каждого файла вызывается функция processFile, которая занимается
  // чтением файла и поиском совпадений с регулярным выражением.
  while ((stringSize = getline(&line, &memlen, file)) != -1) {
    int result = regexec(reg, line, 0, NULL, 0);
    int match = (result == 0 && !flags.v) || (result != 0 && flags.v);

    if (match) {
      coincidence++;
      if (!flags.c && !flags.l) {
        if (flags.o && !flags.v) {
          // Если установлен флаг 'o', выводим совпадения с использованием
          // printMatch
          printMatch(flags, path, lineCount, reg, line);
        } else {
          // Выводим имя файла и номер строки, если соответствующие флаги
          // установлены
          if (!flags.h && argc - optind > 1) {
            printf("%s:", path);
          }
          if (flags.n) {
            printf("%d:", lineCount);
          }
          // Выводим строку целиком
          outline(line, stringSize);
        }
      }
    }
    lineCount++;
  }

  free(line);

  // Выводим количество совпадений, если установлен флаг 'c' и не установлен
  // флаг 'l'
  if (flags.c && !flags.l) {
    if (!flags.h && argc - optind > 1) {
      printf("%s:", path);
    }
    printf("%d\n", coincidence);
  }

  // Если установлен флаг 'l' и есть совпадения, выводим имя файла
  if (flags.l && coincidence > 0) {
    printf("%s\n", path);
  }

  fclose(file);
}

void output(options flags, int argc, char *argv[]) {
  regex_t reg;

  // Компиляция регулярного выражения с учетом установленных флагов
  int error = regcomp(&reg, flags.pattern, REG_EXTENDED | flags.i);
  //   regcomp используется для компиляции регулярного выражения. Флаги
  //   REG_EXTENDED и flags.i (если установлен флаг -i для нечувствительности к
  //   регистру) передаются для настройки поведения регулярного выражения.

  if (error != 0) {
    // Если произошла ошибка при компиляции регулярного выражения, выводим
    // сообщение об ошибке
    char error_message[100];
    regerror(error, &reg, error_message, sizeof(error_message));
    // В случае ошибки компиляции, regerror используется для получения сообщения
    // об ошибке, которое затем выводится на стандартный поток ошибок. Программа
    // завершает свою работу с кодом ошибки после освобождения памяти.
    fprintf(stderr, "ERROR: %s\n", error_message);
    exit(EXIT_FAILURE);
  }

  // for (int i = optind; i < argc; i++) проходит по всем аргументам командной
  // строки, которые являются именами файлов, начиная с optind (индекса первого
  // не-опции аргумента).
  for (int i = optind; i < argc; i++) {
    processFile(flags, argv[i], &reg, argc);
  }

  // Освобождаем ресурсы, связанные с регулярным выражением
  regfree(&reg);
  // Освобождаем память, выделенную под паттерн
  free(flags.pattern);
}
