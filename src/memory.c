#include <stdio.h>

#include "memory.h"

void print_mem(const BFData* data) {
  int max_used_ptr = data->max_used_ptr;
  if (max_used_ptr > BF_CAPACITY)
     max_used_ptr = BF_CAPACITY - 1;
  printf("\nMemory Dump (%d bytes):\n", max_used_ptr + 1);
  for (int i = 0; i < max_used_ptr + 1; i += 1) {
    printf(data->mem_ptr == i ? "[%d] " :  " %d  ", data->memory[i]);
    if ((i+1) % 10 == 0)
      printf("\n");
  }
  printf("\n\n");
}

void hide_mem(const BFData* data) {
  int max_used_ptr = data->max_used_ptr;
  if (max_used_ptr > BF_CAPACITY)
     max_used_ptr = BF_CAPACITY - 1;
  printf(CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE);
  for (int i = 0; i < max_used_ptr + 1; i += 1)
    if ((i+1) % 10 == 0)
      printf(CLEAR_CURR_LINE LINE_UP);
  printf(CLEAR_CURR_LINE LINE_UP CLEAR_CURR_LINE);
}

bool valid_ptr(const BFData* data) {
  return data->mem_ptr >= 0 && data->mem_ptr < BF_CAPACITY;
}

void dump_mem(const BFData* data, const char* filename) {
  FILE* file = fopen(filename, "wb");
  if (file == NULL) {
    fprintf(stderr, "ERROR: cannot dump memory, failed to open file: %s\n", filename);
    return;
  }
  
  // Clamp max used pointer without modifing BFData
  int max_used_ptr = data->max_used_ptr;
  if (max_used_ptr > BF_CAPACITY)
    max_used_ptr = BF_CAPACITY - 1;
  
  size_t written = fwrite(data->memory, 1, max_used_ptr, file);
  if (written != (size_t) max_used_ptr)
    fprintf(stderr, "WARNING: something went wrong while dumping memory to file.\n");
  
  fclose(file);
}