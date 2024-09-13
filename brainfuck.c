#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h>

#define BF_CAPACITY 30000 /*30kB*/
#define BF_LOOP_STACK 1024 /* Max amount of nested loops */

#define CLEAR_CURR_LINE "\033[2K"
#define LINE_UP "\033[F"

typedef struct {
  unsigned char* memory;
  int* loop_stack;

  int loop_ptr;
  int mem_ptr;
  int max_used_ptr;

  bool memdump;
  bool warnings;
} BFData;

void print_mem(BFData* data) {
    if (data->max_used_ptr > BF_CAPACITY) {
      data->max_used_ptr = BF_CAPACITY - 1;
    }
    fprintf(stdout, "\nMemory Dump (%d bytes):\n", data->max_used_ptr + 1);
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
    fprintf(stdout, "\n\n");
}

void hide_mem(BFData* data) {
    fprintf(stdout, CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE);
    for (int i = 0; i < data->max_used_ptr + 1; i += 1)
      if ((i+1) % 10 == 0)
        fprintf(stdout, CLEAR_CURR_LINE LINE_UP);
    printf(CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE);
}

void crash(BFData* data, const char* message, int code) {
  fprintf(stderr, "ERROR: program crashed with code %d because: %s\n", code, message);
  if (data->memdump)
    print_mem(data);
  exit(code);
}

void crash_file(BFData* data, const char* message, int code, const char* filename, int pos) {
  fprintf(stderr, "%s:%d: ERROR: %s\n", filename, pos+1, message);
  fprintf(stderr, "Program crashed with exit code %d\n", code);
  if (data->memdump)
    print_mem(data);
  exit(code);
}

bool valid_ptr(BFData* data) {
  return data->mem_ptr >= 0 && data->mem_ptr < BF_CAPACITY;
}

void interpret(BFData* data, const char* code, const char* filename);

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
  interpret(data, file_buffer, filename);
}

void interpret(BFData* data, const char* code, const char* filename) {
  int len = strlen(code);

  for (int i = 0; i < len; i += 1) {
    char cmd = code[i];
    switch (cmd) {
    case '?': {
      print_mem(data);
      getch();
      hide_mem(data);
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
      char* included_filename = malloc(size);
      included_filename[size-1] = '\0';
      for (int i = start, j = 0; i < end; i += 1, j += 1) {
        included_filename[j] = code[i];
      }
      interpet_file(data, included_filename);
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
    }
  }
}

int main(int argc, char** argv) {
  bool memory_dump = false;
  bool warnings = false;

  // Iterate through arguments and find flags
  int non_flags = 0;
  for (int i = 1; i < argc; i += 1) {
    int arg_len = strlen(argv[i]);
    
    bool is_flag = false;
    if (arg_len > 0 && argv[i][0] == '-')
      is_flag = true;
    if (arg_len > 1 && argv[i][0] == '-' && argv[i][1] == '-')
      is_flag = true;

    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
      fprintf(
        stdout,
        "Usage: %s [options] file...\n"
        "Options:\n"
        "  --help      Display this information.\n"
        "  --version   Display interpreter version information.\n"
        "  --memdump   Print memory dump after program execution.\n"
        "  --warning   Print warnings during program execution.\n",
        argv[0]
      );
      exit(0);
    }
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "--version") == 0) {
      fprintf(stdout, "BrainFuckPlus Interpreter v1.0 by Tymon \"MODERR\" Wozniak\n");
      exit(0);
    }
    if (strcmp(argv[i], "-memdump") == 0 || strcmp(argv[i], "--memdump") == 0) {
      memory_dump = true;
      continue;
    }
    if (strcmp(argv[i], "--warning") == 0 || strcmp(argv[i], "-warning") == 0) {
      warnings = true;
      continue;
    }

    if (!is_flag)
      non_flags++;
    else {
      fprintf(stderr, "ERROR: unknown flag %s. Use --help to see all flags.\n", argv[i]);
      exit(1);
    }
  }

  if (non_flags == 0) {
    fprintf(stderr,
      "Usage: %s [options] <file.bf>\n",
      argv[0]
    );
    exit(EXIT_FAILURE);
  }

  // Allocate all BF memory and stacks
  unsigned char memory[BF_CAPACITY] = {0};
  int loop_stack[BF_LOOP_STACK] = {0};

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

  if (bf_data.memdump)
    print_mem(&bf_data);
  return 0;
}