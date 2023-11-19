#include "blocks.h"
#include "buffer.h"
#include <stdbool.h>
#include <stdio.h>

char *g_buffer = NULL;
int g_bufp = 0;
Block **g_functions = NULL;
int g_funp = 0;

/* Conglomorate of all of the parsing, interpretation, and execution
 * functions. */
void interpret(Block *blocks, bool debug) {

  // Get number of blocks
  int blocks_l = count_blocks(blocks);

  // Allocate buffer and function pool
  if (g_buffer == NULL) {
    g_buffer = (char *)make_buffer(BUFFER_SIZE);
  }
  if (g_functions == NULL) {
    g_functions = (Block **)malloc(sizeof(Block *) * BUFFER_SIZE);
  }

  for (int i = 0; i < blocks_l; i++) {
    Block *current_block = &(blocks[i]);

    switch (current_block->action) {
    case ACT_SET: {

      g_buffer[g_bufp] = *(char *)(current_block->value);

      break;
    }

    case ACT_LEFT: {

      g_bufp -= 1;

      break;
    }

    case ACT_RIGHT: {

      g_bufp += 1;

      break;
    }

    case ACT_JUMP: {

      g_bufp = g_buffer[g_bufp];

      break;
    }

    case ACT_ADD: {

      g_buffer[g_bufp] += 1;

      break;
    }

    case ACT_SUB: {

      g_buffer[g_bufp] -= 1;

      break;
    }

    case ACT_LOOP: {

      while (g_buffer[g_bufp] > 0) {
        interpret((Block *)current_block->value, false);
      }

      break;
    }

    case ACT_AUTO_LOOP: {

      char *iter_count = &(g_buffer[g_bufp]);
      while (*iter_count > 0) {
        interpret((Block *)current_block->value, false);
        *iter_count -= 1;
      }

      break;
    }

    case ACT_FUNCTION: {

      g_functions[g_funp] = current_block->value;
      g_funp++;

      break;
    }

    case ACT_PRINT: {

      printf("%s", &(g_buffer[g_bufp]));

      break;
    }

    case ACT_INPUT: {

      if (scanf(" %c", &(g_buffer[g_bufp])) < 1) {
        g_buffer[g_bufp] = '\0';
      }

      break;
    }

    case ACT_RUN: {

      interpret(g_functions[(int)(g_buffer[g_bufp])], false);

      break;
    }

    case ACT_STRING: {

      char *str = (char *)(current_block->value);
      int start = g_bufp;
      while (str[g_bufp - start] != '\0') {
        g_buffer[g_bufp] = str[g_bufp - start];
        g_bufp++;
      }

      break;
    }

    default: {
      break;
    }
    }

    if (g_bufp < 0) {
      g_bufp = BUFFER_SIZE - 1;
    } else if (g_bufp > BUFFER_SIZE - 1) {
      g_bufp = 0;
    }
  }

  if (debug) {
    // Print the g_buffer
    printf("\n");
    for (int i = 0; i < BUFFER_SIZE; i++) {
      printf("%d ", g_buffer[i]);
    }
    printf("\n");
  }
}

int main(void) {

  // Debug mode?
  printf("Debug mode? (y/N): ");
  char debug_mode;
  if (scanf(" %c", &debug_mode) < 1) {
    return 1;
  }
  debug_mode = debug_mode == 'y' ? true : false;

  // Absorb newline
  getchar();

  // Allow quitting and help
  printf("\nEnter 'h' if you need help, or 'q' to quit the interpreter.\n\n");

  char buf[BUFFER_SIZE];
  for (;;) {

    // Take user input
    printf(">>> ");
    if (fgets(buf, BUFFER_SIZE, stdin) == NULL) {
      continue;
    }

    switch (buf[0]) {
    case 'q':
    case 'Q': {
      free(g_buffer);
      return 0;
    }
    case 'h':
    case 'H': {

      printf("\n");
      printf("A list of characters and their functions:\n");
      printf("\tv | ^  Represent binary 0s and 1s.\n");
      printf("\t< | >  Used to navigate left and right through the buffer.\n");
      printf("\t  ~    Navigates to the position, specified by the current "
             "cell's value, in the buffer.\n");
      printf("\t+ | -  Add 1 / remove 1 from the current cell.\n");
      printf("\t  [    Used to open standard loops. These loops "
             "continue as long as the current cell's value is above 0.\n");
      printf("\t  (    Opening for auto-loops. These "
             "loops remove 1 from the cell they are instanciated at until "
             "mentioned cell's value reaches 0.\n");
      printf("\t  {    Opens a function block. The function data is stored "
             "seperately and assigned an ID. This ID is automatically assigned "
             "to the currently selected cell.\n");
      printf("\t  ;    Signifies the end of a loop, auto loop or function "
             "block. Can also be used to end the program, if used outside of a "
             "nested block.\n");
      printf("\t  #    Shifts through every character in the buffer from the "
             "cell it is called at and prints each cell value until a null "
             "byte is reached. Does NOT change the current cell.\n");
      printf("\t  ?    Takes a single character input from the user and stores "
             "it in the current cell.\n");
      printf("\t  *    Runs the function that matches the ID the current cell "
             "holds, if any.\n");
      printf(
          "\t  \"    Open/close a string. When you declare a string, the "
          "bytes within it are spread across the cells following that of "
          "which you declared the string at. The current cell position is "
          "shifted to the one that houses the last character of the string.\n");
      printf("\n");

      continue;
    }
    }

    // Blockify and check for errors
    Block *blocks = to_blocks(buf);
    char *errors = block_errors(blocks, BUFFER_SIZE);

    if (strlen(errors) != 0) {
      for (int i = 0; i < BUFFER_SIZE; i++) {
        if (errors[i] == '\0') {
          break;
        }
        printf("invalid character '%c'\n", errors[i]);
      }
      continue;
    }

    // Run the code
    interpret(blocks, (bool)debug_mode);

    // Free allocated memory
    free_blocks(blocks);
    free(errors);
  }

  // At the end because it's used during each loop
  free(g_buffer);
  for (int i = 0; i < g_funp; i++) {
    free(g_functions[i]);
  }
  free(g_functions);

  return 0;
}
