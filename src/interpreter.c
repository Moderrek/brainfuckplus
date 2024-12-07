#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "memory.h"
#include "interpreter.h"

bool interpret_file(BFData* data, const char* filename) {
  FILE* file = fopen(filename, "r");
  // Cannot open file
  if (file == NULL) {
    fprintf(stderr, "ERROR: %s: cannot open file: %s\n", filename, strerror(errno));
    return false;
  }

  // Check file size
  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  // Allocate buffer
  char* file_buffer = (char*) malloc(file_size + 1);
  if (!file_buffer) {
    fclose(file);
    fprintf(stderr, "ERROR: %s: cannot allocate %zu bytes.\n", filename, file_size);
    return false;
  }

  // Read whole content into buffer
  size_t read_size = fread(file_buffer, 1, file_size, file);
  if (read_size != file_size) {
    fprintf(stderr, "ERROR: %s: failed to read file content. read %zu bytes, expected %zu bytes", filename, read_size, file_size);
    fclose(file);
    free(file_buffer);
    return false;
  }
  file_buffer[file_size] = '\0';
  fclose(file);

  bool result = interpret(data, file_buffer, file_size, filename);

  free(file_buffer);
  
  return result;
}

bool interpret(BFData* data, const char* code, size_t code_length, const char* filename) {
  for (size_t i = 0; i < code_length; i += 1) {
    switch (code[i]) {
    case '$':
      {
        // Stop execution
        return true;
      }
    case '?':
      {
        print_mem(data);
        fprintf(stdout, "Press enter to continue...");
        getchar();
        hide_mem(data);
        break;
      }
    case '"':
      {
        // Scan for string
        const size_t filename_start = ++i;
        size_t filename_end = i;
        while (i < code_length && code[i] != '"') {
          filename_end++;
          i++;
        }
        const size_t size = filename_end - filename_start;
        // Allocate memory for string
        char* included_filename = (char*)malloc(sizeof(char) * (size + 1));
        if (included_filename == NULL) {
          fprintf(stderr, "%s:%ld ERROR: failed to allocate string\n", filename, i);
          return false;
        }
        // Copy string
        included_filename[size] = '\0';
        for (size_t j = filename_start, k = 0; j < filename_end; j += 1, k += 1) {
          included_filename[k] = code[j];
        }
        // Interpret included file
        if (!interpret_file(data, included_filename)) {
          fprintf(stderr, "%s:%ld: ERROR: failed to include file: %s\n", filename, i, included_filename);
          free(included_filename);
          return false;
        }
        free(included_filename);
        break;
      }
    case '>':
      {
        // Move pointer right
        data->mem_ptr++;
        // Check is pointer out of memory
        if (data->warnings && data->mem_ptr + 1 >= BF_CAPACITY) {
          fprintf(stderr, "%s:%ld: WARNING: pointer go out of memory at char number %ld\n", filename, i, i);
        }
        if (data->mem_ptr > data->max_used_ptr)
          data->max_used_ptr = data->mem_ptr;
        break;
      }
    case '<':
      {
        // Move pointer left
        data->mem_ptr--;
        // Check is pointer out of memory
        if (data->warnings && data->mem_ptr < 0) {
          fprintf(stderr, "%s:%ld WARNING: pointer go behind the memory, can cause fatal error.\n", filename, i);
        }
        break;
      }
    case '+':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%ld ERROR: tried to increment cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        if (data->memory[data->mem_ptr] == 255) {
          fprintf(stderr, "%s:%ld: ERROR: tried to increment maximal value of single byte, ptr = %d\n",
                  filename, i, data->mem_ptr);
          return false;
        }
        data->memory[data->mem_ptr]++;
        break;
      }
    case '-':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%ld ERROR: tried to decrement cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        if (data->memory[data->mem_ptr] == 0) {
          fprintf(stderr, "%s:%ld: ERROR: tried to decrement minimal value of single byte, ptr = %d\n",
                  filename, i, data->mem_ptr);
          return false;
        }
        data->memory[data->mem_ptr]--;
        break;
      }
    case ',':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%ld ERROR: tried to write to cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        if (data->debug)
          printf("char > ");
        data->memory[data->mem_ptr] = getchar();
        break;
      }
    case ';':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%ld ERROR: tried to write to cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        if (data->debug)
          printf("uint8 > ");
        int temp = 0;
        int scanned = scanf("%d", &temp);
        if (scanned != 1) {
          fprintf(stderr, "%s:%ld ERROR: failed to read uint8 value from standard input.\n", filename, i);
          return false;
        }
        if (temp >= 0 && temp <= 255) {
          data->memory[data->mem_ptr] = (unsigned char) temp;
          break;
        }
        fprintf(stderr, "%s:%ld ERROR: entered value (%d) is out of range 0-255!\n", filename, i, temp);
        return false;
        break;
      }
    case '.':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%ld ERROR: tried to write to cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        putchar(data->memory[data->mem_ptr]);
        break;
      }
    case ':':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%ld ERROR: tried to write to cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        printf("%d", data->memory[data->mem_ptr]);
        break;
      }
    case '[':
      {
        if (data->memory[data->mem_ptr] == 0) {
          int loop_nesting = 1;
          while (loop_nesting > 0) {
            i++;
            if (i >= code_length) {
              fprintf(stderr, "%s:%ld: ERROR: unbalanced '['\n", filename, i);
              return false;
            }
            if (code[i] == '[') loop_nesting++;
            else if (code[i] == ']') loop_nesting--;
          }
        }
        else {
          if (data->loop_ptr + 1 >= BF_LOOP_STACK) {
            fprintf(stderr, "%s:%ld: ERROR: too many nested loops\n", filename, i);
            return false;
          }
          data->loop_stack[++data->loop_ptr] = i;
        }
        break;
      }
    case ']':
      {
        if (data->loop_ptr == -1) {
          fprintf(stderr, "%s:%ld: ERROR: unmatched ']'\n", filename, i);
          return false;
        }
        if (data->memory[data->mem_ptr] != 0) {
          i = data->loop_stack[data->loop_ptr];
        }
        else {
          data->loop_ptr--;
        }
        break;
      }
    default:
      {
        break;
      }
    }
  }
  return true;
}
