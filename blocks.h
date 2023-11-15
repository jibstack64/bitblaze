/* Utilities for breaking down bitblaze code into "blocks". These blocks
 * essentially tell the interpreter what to do and where when
 * interpreted. */

#ifndef BLOCKS_H
#define BLOCKS_H

#include "bin.h"
#include "bitblaze.h"
#include "pointer.h"
#include <stdlib.h>
#include <string.h>

/* Represent different actions (set, jump, etc). */
typedef enum {
  ACT_SET,
  ACT_LEFT,
  ACT_RIGHT,
  ACT_JUMP,
  ACT_ADD,
  ACT_SUB,
  ACT_LOOP,
  ACT_AUTO_LOOP,
  ACT_FUNCTION,
  ACT_PRINT,
  ACT_INPUT,
  ACT_RUN,
  ACT_STRING,

  ACT_END,
  ACT_ERROR
} Action;

/* Associates an action and its necessary(?) value. */
typedef struct {
  Action action;
  void *value; // If any
} Block;

/* Converts an `Action` to it's title.
 * Mostly useful for debugging. */
const char *action_str(Action a) {
  switch (a) {
  case ACT_SET:
    return "set value";
  case ACT_LEFT:
    return "shift left";
  case ACT_RIGHT:
    return "shift right";
  case ACT_JUMP:
    return "jump";
  case ACT_ADD:
    return "add 1";
  case ACT_SUB:
    return "subtract 1";
  case ACT_LOOP:
    return "[loop]";
  case ACT_AUTO_LOOP:
    return "(auto loop)";
  case ACT_FUNCTION:
    return "{function}";
  case ACT_PRINT:
    return "print";
  case ACT_INPUT:
    return "input";
  case ACT_RUN:
    return "run function";
  case ACT_STRING:
    return "string";
  case ACT_END:
    return "eo(f/s)";
  case ACT_ERROR:
    return "error";
  }
  return "???";
}

/* Frees all `blocks` and their data, including nested loop blocks. */
void free_blocks(Block *blocks) {
  int position = 0;
  while (blocks[position].action != ACT_END) {
    Block *block = &(blocks[position]);

    if (block->action == ACT_LOOP || block->action == ACT_AUTO_LOOP) {
      free_blocks((Block *)(block->value));

    } else if (block->value != NULL) {
      free(block->value);
    }

    position++;
  }
  free(blocks);
}

/* Seeks to the end of `blocks` and returns the resulting length (including the
 * end block). */
int count_blocks(Block *blocks) {
  int position = 0;
  while (blocks[position].action != ACT_END) {
    position++;
  }
  return position;
}

/* Prints `blocks` to the screen, increasing the `tab` with each level of loop
 * nesting. */
void print_blocks(Block *blocks, int tab) {
  int i = 0;
  while (blocks[i].action != ACT_END) {
    for (int t = 0; t < tab; t++) {
      printf("\t");
    }
    printf("%s\n", action_str(blocks[i].action));
    if (blocks[i].action == ACT_LOOP || blocks[i].action == ACT_AUTO_LOOP) {
      print_blocks((Block *)(blocks[i].value), tab + 1);
    }
    i++;
  }
}

/* Finds all errors in `blocks`. */
char *block_errors(Block *blocks, int chs) {
  char *errors = (char *)malloc(sizeof(char) * (chs + 1));
  int errp = 0;

  int blocks_l = count_blocks(blocks);
  for (int i = 0; i < blocks_l; i++) {
    switch (blocks[i].action) {
    case ACT_LOOP:
    case ACT_AUTO_LOOP: {

      char *nested = block_errors((Block *)(blocks[i].value),
                                  chs - 1); // -1 for loop start
      int pos = 0;
      while (nested[pos] != '\0') {
        errors[errp + pos] = nested[pos];
        pos++;
      }
      errp += pos;

      free(nested);

      break;
    }
    case ACT_ERROR: {
      errors[errp] = *(char *)(blocks[i].value);
      errp++;
      break;
    }
    default:
      break;
    }
  }

  // Terminate for processing
  errors[errp] = '\0';

  return errors;
}

/* Attempts to parse `code` to `Block`s. */
Block *to_blocks(const char *code) {
  int code_l = strlen(code);
  Block *blocks = (Block *)malloc(sizeof(Block) * (code_l + 1));

  int block_pos = 0;

  for (int i = 0; i < code_l; i++) {

    // Get the current char and block
    char c = code[i];
    Block *current_block = &(blocks[block_pos]);

    switch (c) {

    case KEY_BIT_0:
    case KEY_BIT_1: {
      current_block->action = ACT_SET;

      // Seek to the end of the provided binary data
      char *buf = (char *)malloc(sizeof(char) * 9);
      int pos;
      for (pos = 0; pos < 8; pos++) {
        char ch = code[i + pos];

        if (!(ch == KEY_BIT_0 || ch == KEY_BIT_1)) {
          break;
        }

        buf[pos] = ch == KEY_BIT_0 ? '0' : '1';
      }

      i += pos - 1;

      // Make valid C-string, parse bin and free
      buf[8] = '\0';
      char binary = interpret_bin(buf);
      free(buf);

      current_block->value = make_pointer((int)binary);

      break;
    }

    case KEY_LEFT:

      current_block->action = ACT_LEFT;

      break;

    case KEY_RIGHT:

      current_block->action = ACT_RIGHT;

      break;

    case KEY_JUMP:

      current_block->action = ACT_JUMP;

      break;

    case KEY_ADD:

      current_block->action = ACT_ADD;

      break;

    case KEY_SUB:

      current_block->action = ACT_SUB;

      break;

    case KEY_LOOP_OPEN:
    case KEY_AUTO_LOOP_OPEN:
    case KEY_FUNCTION_OPEN: {

      if (c == KEY_LOOP_OPEN) {
        current_block->action = ACT_LOOP;
      } else if (c == KEY_AUTO_LOOP_OPEN) {
        current_block->action = ACT_AUTO_LOOP;
      } else {
        current_block->action = ACT_FUNCTION;
      }

      // Parse following blocks separately
      current_block->value = to_blocks(&(code[i + 1]));

      // Skip the chars that the loop went over
      Block *rb = (Block *)current_block->value;
      int x = 0;
      while (rb[x].action != ACT_END) {
        x++;
      }
      i += *(int *)(rb[x].value) + 1;

      break;
    }

    case KEY_LOOP_CLOSE:
    case KEY_AUTO_LOOP_CLOSE:
    case KEY_FUNCTION_CLOSE:
    case KEY_CLOSE: {

      // Allows KEY_X_LOOP_OPEN to know how many chars were gone through
      current_block->action = ACT_END;
      current_block->value = make_pointer(i);

      // Return blocks to allow logic in KEY_X_LOOP_OPEN block to work
      return blocks;
    }

    case KEY_PRINT:

      current_block->action = ACT_PRINT;

      break;

    case KEY_INPUT:

      current_block->action = ACT_INPUT;

      break;

    case KEY_RUN:

      current_block->action = ACT_RUN;

      break;

    case KEY_STRING:

      current_block->action = ACT_STRING;
      current_block->value =
          (char *)malloc(sizeof(char) * code_l); // No +1 cus covered by " char

      int pos;
      for (pos = 1; pos < code_l; pos++) {
        char ch = code[i + pos];

        if (ch == '"') {
          break;
        }

        ((char *)(current_block->value))[pos - 1] = ch;
      }

      // NULL-terimate at appropriate position
      ((char *)(current_block->value))[pos] = '\0';

      i += pos;

      break;

    case ' ': // Ignore spaces and newlines
    case '\n':
      block_pos--;

      break;

    default: {

      current_block->action = ACT_ERROR;
      current_block->value = make_pointer(c);

      break;
    }
    }

    // Go to the next block
    block_pos++;
  }

  blocks[block_pos].action = ACT_END;

  return blocks;
}

#endif