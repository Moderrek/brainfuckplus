#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define BF_CAPACITY 30000 /*30kB*/
#define BF_LOOP_STACK 1024 /* Max amount of nested loops */

typedef struct {
  unsigned char* memory;
  int* loop_stack;

  int loop_ptr;
  int mem_ptr;
  int max_used_ptr;

  bool memdump;
  bool warnings;
} BFData;

bool valid_ptr(BFData* data) {
  return data->mem_ptr >= 0 && data->mem_ptr < BF_CAPACITY;
}

void print_mem(BFData* data) {
  if (data->memdump) {
    if (data->max_used_ptr > BF_CAPACITY) {
      data->max_used_ptr = BF_CAPACITY - 1;
    }
    fprintf(stdout, "\n\nMemory Dump (%d cells):\n", data->max_used_ptr + 1);
    
    for (int i = 0; i < data->max_used_ptr + 1; i += 1) {
      if (data->mem_ptr == i) {
        fprintf(stdout, "[%d] ", data->memory[i]);
      } else {
        fprintf(stdout, " %d  ", data->memory[i]);
      }
      
      if ((i+1) % 10 == 0) {
        fprintf(stdout, "\n");
      }
    }
  }
}

void interpret(BFData* data, const char* code);

void interpet_file(BFData* data, const char* filename) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "ERROR: cannot open file: %s\n", filename);
    exit(1);
  }
  char file_buffer[4096] = {0};
  
  fread(file_buffer, sizeof(char), 4096, file);
  fclose(file);

  file_buffer[4095] = '\0';
  interpret(data, file_buffer);
}

void interpret(BFData* data, const char* code) {
  int len = strlen(code);

  for (int i = 0; i < len; i += 1) {
    char cmd = code[i];
    switch (cmd) {
    case '"':
      int start = ++i;
      int end = i;
      while (i < len && code[i] != '"') {
        end++;
        i++;
      }
      int size = end - start + 1;
      char* filename = malloc(size);
      filename[size-1] = '\0';
      for (int i = start, j = 0; i < end; i += 1, j += 1) {
        filename[j] = code[i];
      }
      interpet_file(data, filename);
      break;
    case '>':
      // Move pointer right
      data->mem_ptr++;
      // Check is pointer out of memory
      if (data->warnings && data->mem_ptr + 1 >= BF_CAPACITY) {
        fprintf(stderr, "WARNING: pointer go out of memory at char number %d\n", i);
      }
      if (data->mem_ptr > data->max_used_ptr)
        data->max_used_ptr = data->mem_ptr;
      break;
    case '<':
      // Move pointer left
      data->mem_ptr--;
      // Check is pointer out of memory
      if (data->warnings && data->mem_ptr < 0) {
        fprintf(stderr, "WARNING: pointer go behing the memory, can cause fatal error. @ %d char\n", i);
      }
      break;
    case '+':
      if (data->memory[data->mem_ptr] == 255) {
        fprintf(stderr, "ERROR: reached max value of single byte at %d. Use --memdump to view memory\n", data->mem_ptr);
        goto end;
      }
      data->memory[data->mem_ptr]++;
      break;
    case '-':
      if (data->memory[data->mem_ptr] == 0) {
        fprintf(stderr, "ERROR: reached min value of single byte at %d. Use --memdump to view memory\n", data->mem_ptr);
        goto end;
      }
      data->memory[data->mem_ptr]--;
      break;
    case ',':
      if (!valid_ptr(data)) {
        fprintf(stderr, "ERROR: tried to write to cell which is out of memory\n");
        goto end;
      }
      fprintf(stdout, "> ");
      fscanf(stdin, "%c", data->memory + data->mem_ptr);
      break;
    case ';':
      if (!valid_ptr(data)) {
        fprintf(stderr, "ERROR: tried to write to cell which is out of memory\n");
        goto end;
      }
      fprintf(stdout, "> ");
      fscanf(stdin, "%d", data->memory + data->mem_ptr);
      break;
    case '.':
      if (!valid_ptr(data)) {
        fprintf(stderr, "ERROR: tried to read cell which is out of memory\n");
        goto end;
      }
      fprintf(stdout, "%c", data->memory[data->mem_ptr]);
      break;
    case ':':
      if (!valid_ptr(data)) {
        fprintf(stderr, "ERROR: tried to read cell which is out of memory\n");
        goto end;
      }
      fprintf(stdout, "%d", data->memory[data->mem_ptr]);
      break;
    case '[':
      if (data->memory[data->mem_ptr] == 0) {
        int loop_nesting = 1;
        while (loop_nesting > 0) {
          i++;
          if (i >= len) {
            fprintf(stderr, "ERROR: unbalanced '['\n");
            goto end;
          }
          if (code[i] == '[') loop_nesting++;
          else if (code[i] == ']') loop_nesting--;
        }
      } else {
        if (data->loop_ptr + 1 >= BF_LOOP_STACK) {
          fprintf(stderr, "ERROR: too many nested loops\n");
          goto end;
        }
        data->loop_stack[++data->loop_ptr] = i;
      }
      break;
    case ']':
      if (data->loop_ptr == -1) {
        fprintf(stderr, "ERROR: unmatched ']'\n");
        goto end;
      }
      if (data->memory[data->mem_ptr] != 0) {
        i = data->loop_stack[data->loop_ptr];
      } else {
        data->loop_ptr--;
      }
      break;
    }
  }

end:
}

int main(int argc, char** argv) {
  if (argc == 1) {
    fprintf(stderr, "ERROR: no input file\n");
    return 1;
  }
  bool memory_dump = false;
  bool warnings = false;
  for (int i = 1; i < argc; i += 1) {
    if (strcmp(argv[i], "-memdump") == 0 || strcmp(argv[i], "--memdump") == 0) {
      memory_dump = true;
      continue;
    }
    if (strcmp(argv[i], "--warning") == 0 || strcmp(argv[i], "-warning") == 0) {
      warnings = true;
      continue;
    }
  }

  // Allocate all BF memory and stacks
  unsigned char memory[BF_CAPACITY] = {0};
  int loop_stack[BF_LOOP_STACK];

  BFData bf_data = {
    .memory = memory,
    .loop_stack = loop_stack,

    .loop_ptr = -1,
    .mem_ptr = 0,
    .max_used_ptr = 0,

    .memdump = memory_dump,
    .warnings = warnings
  };

  for (int i = 1; i < argc; i += 1) {
    if (strcmp(argv[i], "-memdump") == 0 || strcmp(argv[i], "--memdump") == 0)
      continue;
    if (strcmp(argv[i], "--warning") == 0 || strcmp(argv[i], "-warning") == 0)
      continue;
    const char* filename = argv[i];
    interpet_file(&bf_data, filename);
  }

  print_mem(&bf_data);
  return 0;
}