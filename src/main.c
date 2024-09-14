#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "interpreter.h"
#include "memory.h"

int main(const int argc, char** argv) {

  BFData bf_data = {
    .memory = NULL,
    .loop_stack = NULL,

    .loop_ptr = -1,
    .mem_ptr = 0,
    .max_used_ptr = 0,

    .debug = false,
    .memdump = false,
    .memdump_file = NULL,
    .warnings = false
  };

  bool* is_flag = (bool*) malloc(sizeof(bool) * argc);
  memset(is_flag, 0, sizeof(bool) * argc);
  // Iterate through arguments and find flags
  int non_flags = 0;
  for (int i = 1; i < argc; i += 1) {
    const int arg_len = strlen(argv[i]);
    
    if (arg_len > 0 && argv[i][0] == '-')
      is_flag[i] = true;
    if (arg_len > 1 && argv[i][0] == '-' && argv[i][1] == '-')
      is_flag[i] = true;

    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
      printf(
        "Usage: %s [options] file...\n"
        "Options:\n"
        "  --help                    Display this information.\n"
        "  --version                 Display interpreter version information.\n"
        "  --warning                 Print warnings during program execution.\n"
        "  --debug                   Print debug information during program execution.\n"
        "  --memdump                 Print memory dump after program execution.\n"
        "  --memdumpfile=<filename>  Dump memory into file after program execution.\n",
        argv[0]
      );
      free(is_flag);
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "--version") == 0) {
      printf("BrainFuckPlus Interpreter v1.1 by Tymon \"MODERR\" Wozniak\n");
      free(is_flag);
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[i], "-memdump") == 0 || strcmp(argv[i], "--memdump") == 0) {
      bf_data.memdump = true;
      continue;
    }
    if (strncmp(argv[i], "--memdumpfile=", 14) == 0) {
      bf_data.memdump_file = argv[i] + 14;
      continue;
    }
    if (strncmp(argv[i], "-memdumpfile=", 13) == 0) {
      bf_data.memdump_file = argv[i] + 13;
      continue;
    }
    if (strcmp(argv[i], "--warning") == 0 || strcmp(argv[i], "-warning") == 0 || strcmp(argv[i], "-w") == 0) {
      bf_data.warnings = true;
      continue;
    }
    if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-debug") == 0 || strcmp(argv[i], "-d") == 0) {
      bf_data.debug = true;
      continue;
    }

    if (!is_flag[i])
      non_flags++;
    else {
      fprintf(stderr, "ERROR: unknown flag %s. Use --help to see all flags.\n", argv[i]);
      free(is_flag);
      return EXIT_FAILURE;
    }
  }

  if (non_flags == 0) {
    fprintf(stderr,
      "Usage: %s [options] <file>...\n",
      argv[0]
    );
    free(is_flag);
    return EXIT_FAILURE;
  }

  // Allocate all BF memory and stacks
  unsigned char memory[BF_CAPACITY] = {0};
  int loop_stack[BF_LOOP_STACK] = {0};
  
  bf_data.memory = memory;
  bf_data.loop_stack = loop_stack;

  for (int i = 1; i < argc; i += 1) {
    if (is_flag[i]) continue;
    
    const char* filename = argv[i];
    if (!interpret_file(&bf_data, filename)) {
      fprintf(stderr, "ERROR: cannot interpret file: %s\n", filename);
      break;
    }
  }

  if (bf_data.memdump)
    print_mem(&bf_data);
  if (bf_data.memdump_file != NULL)
    dump_mem(&bf_data, bf_data.memdump_file);
  free(is_flag);
  return EXIT_SUCCESS;
}
