#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LIBARGV_IMPLEMENTATION
#include "../../c-assets/libargv/standalone_lib/libargv.h"

#define IO_BUFFER_SIZE (64 * 1024)

bool is_printable(unsigned char c, bool include_all_whitespace) {
  if (c >= 32 && c <= 126) return true;
  if (include_all_whitespace) {
    return (c >= 9 && c <= 13);
  }
  return false;
}

void print_offset(size_t offset, char radix) {
  switch (radix) {
    case 'd': printf("%7zu ", offset); break;
    case 'x': printf("%7zx ", offset); break;
    case 'o': printf("%7zo ", offset); break;
    default: break;
  }
}

void emit_string(const char *file_name, bool print_file_name, size_t offset, char radix,
    const char *str, size_t len, const char *separator) {
  if (print_file_name && file_name) {
    printf("%s: ", file_name);
  }
  print_offset(offset, radix);
  fwrite(str, 1, len, stdout);
  if (separator == NULL) {
    putchar('\n');
  } else {
    fputs(separator, stdout);
  }
}

int main(int argc, char **argv) {
  const char **source_file = NULL;
  size_t source_file_count = 0;
  const char *output_separator = NULL;
  char radix = ' ';
  bool include_all_whitespace = false;
  bool print_file_name = false;
  int min_len = 4;

  CliParser parser;
  argv_init(&parser, argv[0]);
  argv_bind_rest(&parser, "source", &source_file, &source_file_count, false, NULL, "Source files.");
  argv_bind_int(&parser, "-n", "--bytes", &min_len, false, NULL, "Minimum bytes in a string.");
  argv_bind_str(&parser, "-s", "--output-separator", &output_separator, false, NULL, "Custom separator.");
  argv_bind_char(&parser, "-t", "--radix", &radix, false, "d,x,o", "Print offset (d, o, x).");
  argv_bind_bool(&parser, "-w", "--include-all-whitespace", &include_all_whitespace, "Include newlines/whitespace.");
  argv_bind_bool(&parser, "-f", "--print-file-name", &print_file_name, "Print file name.");

  CliStatus status = argv_parse(&parser, argc, argv);
  if (status == ARGV_HELP_REQUESTED) {
    argv_free(&parser);
    return 0;
  }
  if (status != ARGV_OK) {
    fprintf(stderr, "Error in command line: %s\nUse '%s --help'.\n", argv_get_error(&parser), argv[0]);
    argv_free(&parser);
    return 1;
  }

  if (min_len <= 0) {
    fprintf(stderr, "[ERROR] Minimum bytes length must be greater than 0.\n");
    argv_free(&parser);
    return 1;
  }

  size_t capacity = 1024;
  char *str = malloc(capacity);
  unsigned char *buffer = malloc(IO_BUFFER_SIZE);

  if (!str || !buffer) {
    fprintf(stderr, "[ERROR] Memory allocation failed.\n");
    free(str);
    free(buffer);
    argv_free(&parser);
    return 1;
  }

  size_t loops = (source_file_count == 0) ? 1 : source_file_count;

  for (size_t i = 0; i < loops; i++) {
    const char *filename = (source_file_count == 0) ? NULL : source_file[i];
    FILE *file = stdin;

    if (filename != NULL && strcmp(filename, "-") != 0) {
      file = fopen(filename, "rb");
      if (!file) {
        fprintf(stderr, "[ERROR] Could not open file %s.\n", filename);
        continue;
      }
    }

    size_t num_bytes = 0;
    size_t total_bytes_read = 0;
    size_t char_count = 0;

    while ((num_bytes = fread(buffer, 1, IO_BUFFER_SIZE, file)) > 0) {
      for (size_t j = 0; j < num_bytes; j++) {
        total_bytes_read++;

        if (is_printable(buffer[j], include_all_whitespace)) {
          if (char_count >= capacity) {
            capacity *= 2; 
            char *tmp = realloc(str, capacity);
            if (!tmp) {
              fprintf(stderr, "[ERROR] Out of memory.\n");
              if (file != stdin) fclose(file);
              free(str);
              free(buffer);
              argv_free(&parser);
              return 1;
            }
            str = tmp;
          }
          str[char_count++] = (char)buffer[j];
        } else {
          if (char_count >= (size_t)min_len) {
            emit_string(filename, print_file_name,
                total_bytes_read - 1 - char_count, radix,
                str, char_count, output_separator);
          }
          char_count = 0;
        }
      }
    }

    if (char_count >= (size_t)min_len) {
      emit_string(filename, print_file_name,
          total_bytes_read - char_count, radix,
          str, char_count, output_separator);
    }

    if (file != stdin) {
      fclose(file);
    }
  }

  free(str);
  free(buffer);
  argv_free(&parser);
  return 0;
}
