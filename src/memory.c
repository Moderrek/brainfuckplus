#include <stdio.h>

#include "memory.h"

void print_mem(BFData* data) {
  if (data->max_used_ptr > BF_CAPACITY)
    data->max_used_ptr = BF_CAPACITY - 1;
  printf("\nMemory Dump (%d bytes):\n", data->max_used_ptr + 1);
  for (int i = 0; i < data->max_used_ptr + 1; i += 1) {
    printf(data->mem_ptr == i ? "[%d] " :  " %d  ", data->memory[i]);
    if ((i+1) % 10 == 0)
      printf("\n");
  }
  printf("\n\n");
}

void hide_mem(BFData* data) {
  printf(CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE);
  for (int i = 0; i < data->max_used_ptr + 1; i += 1)
    if ((i+1) % 10 == 0)
      printf(CLEAR_CURR_LINE LINE_UP);
  printf(CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE);
}

bool valid_ptr(BFData* data) {
  return data->mem_ptr >= 0 && data->mem_ptr < BF_CAPACITY;
}

void dump_mem(BFData* data, const char* filename) {
  FILE* file = fopen(filename, "wb");
  if (file == NULL) {
    fprintf(stderr, "ERROR: cannot dump memory, failed to open file: %s\n", filename);
    return;
  }
  for (int i = 0; i < data->max_used_ptr + 1; i += 1) {
    fprintf(file, "%c", data->memory[i]);
  }
  fclose(file);
}