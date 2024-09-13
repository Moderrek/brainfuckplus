#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h>

#include "interpreter.h"

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
      printf(
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
    if (strcmp(argv[i], "--warning") == 0 || strcmp(argv[i], "-warning") == 0 || strcmp(argv[i], "-w") == 0) {
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
    if (strcmp(argv[i], "--warning") == 0 || strcmp(argv[i], "-warning") == 0 || strcmp(argv[i], "-w") == 0)
      continue;
    const char* filename = argv[i];
    bool success = interpret_file(&bf_data, filename);
    if (!success) {
      fprintf(stderr, "ERROR: cannot interpret file: %s\n", filename);
      break;
    }
  }

  if (bf_data.memdump)
    print_mem(&bf_data);
  return EXIT_SUCCESS;
}