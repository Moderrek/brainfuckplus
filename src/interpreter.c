#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "memory.h"
#include "interpreter.h"

bool interpret_file(BFData* data, const char* filename) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "ERROR: %s: cannot open file: %s\n", filename, strerror(errno));
    return false;
  }
  // TODO: dynamic buffer  
  char file_buffer[4096] = {0};
  fread(file_buffer, sizeof(char), 4096, file);
  fclose(file);

  file_buffer[4095] = '\0';
  return interpret(data, file_buffer, filename);
}

bool interpret(BFData* data, const char* code, const char* filename) {
  const int len = strlen(code);
  for (int i = 0; i < len; i += 1) {
    switch (code[i]) {
    case '$':
      {
        // Stop execution
        return true;
      }
    case '?':
      {
        print_mem(data);
        system("pause");
        hide_mem(data);
        break;
      }
    case '"':
      {
        // Scan for string
        const int filename_start = ++i;
        int filename_end = i;
        while (i < len && code[i] != '"') {
          filename_end++;
          i++;
        }
        const int size = filename_end - filename_start;
        // Allocate memory for string
        char* included_filename = (char*)malloc(sizeof(char) * (size + 1));
        if (included_filename == NULL) {
          fprintf(stderr, "%s:%d ERROR: failed to allocate string\n", filename, i);
          return false;
        }
        // Copy string
        included_filename[size] = '\0';
        for (int j = filename_start, k = 0; j < filename_end; j += 1, k += 1) {
          included_filename[k] = code[j];
        }
        // Interpret included file
        if (!interpret_file(data, included_filename)) {
          fprintf(stderr, "%s:%d: ERROR: failed to include file: %s\n", filename, i, included_filename);
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
          fprintf(stderr, "%s:%d: WARNING: pointer go out of memory at char number %d\n", filename, i, i);
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
          fprintf(stderr, "%s:%d WARNING: pointer go behind the memory, can cause fatal error.\n", filename, i);
        }
        break;
      }
    case '+':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%d ERROR: tried to increment cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        if (data->memory[data->mem_ptr] == 255) {
          fprintf(stderr, "%s:%d: ERROR: tried to increment maximal value of single byte, ptr = %d\n",
                  filename, i, data->mem_ptr);
          return false;
        }
        data->memory[data->mem_ptr]++;
        break;
      }
    case '-':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%d ERROR: tried to decrement cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        if (data->memory[data->mem_ptr] == 0) {
          fprintf(stderr, "%s:%d: ERROR: tried to decrement minimal value of single byte, ptr = %d\n",
                  filename, i, data->mem_ptr);
          return false;
        }
        data->memory[data->mem_ptr]--;
        break;
      }
    case ',':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%d ERROR: tried to write to cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
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
          fprintf(stderr, "%s:%d ERROR: tried to write to cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        if (data->debug)
          printf("uint8 > ");
        scanf_s("%d", data->memory + data->mem_ptr);
        break;
      }
    case '.':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%d ERROR: tried to write to cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
          return false;
        }
        putchar(data->memory[data->mem_ptr]);
        break;
      }
    case ':':
      {
        if (!valid_ptr(data)) {
          fprintf(stderr, "%s:%d ERROR: tried to write to cell which is out of memory, ptr = %d\n", filename, i, data->mem_ptr);
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
            if (i >= len) {
              fprintf(stderr, "%s:%d: ERROR: unbalanced '['\n", filename, i);
              return false;
            }
            if (code[i] == '[') loop_nesting++;
            else if (code[i] == ']') loop_nesting--;
          }
        }
        else {
          if (data->loop_ptr + 1 >= BF_LOOP_STACK) {
            fprintf(stderr, "%s:%d: ERROR: too many nested loops\n", filename, i);
            return false;
          }
          data->loop_stack[++data->loop_ptr] = i;
        }
        break;
      }
    case ']':
      {
        if (data->loop_ptr == -1) {
          fprintf(stderr, "%s:%d: ERROR: unmatched ']'\n", filename, i);
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
