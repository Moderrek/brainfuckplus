#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "crash.h"
#include "interpreter.h"

void action_breakpoint(BFData* data) {
  print_mem(data);
  system("pause");
  hide_mem(data);
}

bool interpret_file(BFData* data, const char* filename) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "ERROR: cannot open file: %s\n", filename);
    return false;
  }
  char file_buffer[4096] = {0};
  fread(file_buffer, sizeof(char), 4096, file);
  fclose(file);

  file_buffer[4095] = '\0';
  interpret(data, file_buffer, filename);
  return true;
}

bool interpret(BFData* data, const char* code, const char* filename) {
  int len = strlen(code);

  for (int i = 0; i < len; i += 1) {
    char cmd = code[i];
    switch (cmd) {
    case '?': {
      action_breakpoint(data);
      break;
    }
    case '"': {
      int start = ++i;
      int end = i;
      while (i < len && code[i] != '"') {
        end++;
        i++;
      }
      int size = end - start + 1;
      char* included_filename = (char*) malloc(size);
      if (included_filename == NULL) {
        fprintf(stderr, "ERROR: failed to allocate string\n");
        return false;
      }
      included_filename[size-1] = '\0';
      for (int i = start, j = 0; i < end; i += 1, j += 1) {
        included_filename[j] = code[i];
      }
      bool success = interpret_file(data, included_filename);
      if (!success) {
        fprintf(stderr, "ERROR: failed to import file: %s\n", included_filename);
        free(included_filename);
        return false;
      }
      free(included_filename);
      break;
    }
    case '>': {
      // Move pointer right
      data->mem_ptr++;
      // Check is pointer out of memory
      if (data->warnings && data->mem_ptr + 1 >= BF_CAPACITY) {
        fprintf(stderr, "WARNING: pointer go out of memory at char number %d\n", i);
      }
      if (data->mem_ptr > data->max_used_ptr)
        data->max_used_ptr = data->mem_ptr;
      break;
    }
    case '<': {
      // Move pointer left
      data->mem_ptr--;
      // Check is pointer out of memory
      if (data->warnings && data->mem_ptr < 0) {
        fprintf(stderr, "WARNING: pointer go behing the memory, can cause fatal error. @ %d char\n", i);
      }
      break;
    }
    case '+': {
      if (data->memory[data->mem_ptr] == 255) {
        crash_file(data, 
          data->memdump ? 
          "reached maximal value of single byte" : 
          "reached maximal value of single byte. Use --memdump to view memory",
          1, filename, i
        );
      }
      data->memory[data->mem_ptr]++;
      break;
    }
    case '-': {
      if (data->memory[data->mem_ptr] == 0) {
        crash_file(data, 
          data->memdump ? 
          "reached minimal value of single byte" : 
          "reached minimal value of single byte. Use --memdump to view memory",
          1, filename, i
        );
      }
      data->memory[data->mem_ptr]--;
      break;
    }
    case ',': {
      if (!valid_ptr(data))
        crash_file(data, "tried to write to cell which is out of memory", 1, filename, i);
      data->memory[data->mem_ptr] = getchar(); 
      break;
    }
    case ';': {
      if (!valid_ptr(data))
        crash_file(data, "tried to write to cell which is out of memory", 1, filename, i);
      fprintf(stdout, "number > ");
      scanf("%d", data->memory + data->mem_ptr);
      break;
    }
    case '.': {
      if (!valid_ptr(data)) {
        crash_file(data, "tried to read cell which is out of memory", 1, filename, i);
      }
      putchar(data->memory[data->mem_ptr]);
      break;
    }
    case ':': {
      if (!valid_ptr(data)) {
        crash_file(data, "tried to read cell which is out of memory", 1, filename, i);
      }
      fprintf(stdout, "%d", data->memory[data->mem_ptr]);
      break;
    }
    case '[': {
      if (data->memory[data->mem_ptr] == 0) {
        int loop_nesting = 1;
        while (loop_nesting > 0) {
          i++;
          if (i >= len) {
            crash_file(data, "unbalanced '['", 1, filename, i);
          }
          if (code[i] == '[') loop_nesting++;
          else if (code[i] == ']') loop_nesting--;
        }
      } else {
        if (data->loop_ptr + 1 >= BF_LOOP_STACK) {
          crash_file(data, "too many nested loops", 1, filename, i);
        }
        data->loop_stack[++data->loop_ptr] = i;
      }
      break;
    }
    case ']': {
      if (data->loop_ptr == -1) {
        crash_file(data, "unmatched ']'", 1, filename, i);
      }
      if (data->memory[data->mem_ptr] != 0) {
        i = data->loop_stack[data->loop_ptr];
      } else {
        data->loop_ptr--;
      }
      break;
    }
      default: {
      break;
    };
    }
  }
  return true;
}