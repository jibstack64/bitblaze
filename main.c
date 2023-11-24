#include "blocks.h"
#include "buffer.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _WIN32
#include <ncursesw/ncurses.h>
#else
#include <ncurses.h>
#endif
#include <math.h>

char *g_buffer = NULL;
int g_bufp = 0;
Block **g_functions = NULL;
int g_funp = 0;

void interpret(Block*, bool, int);
void help(struct option[]);
void manual(void);
void draw_gbuffer(Block*);
char* read_file(char* fn);

int main(int argc, char** argv) {
	
	struct option long_options[] = {

		/* code input methods */
		{ "code", required_argument, NULL, 'c' },
		{ "sleep", required_argument, NULL, 's' },

		/* flags */
		{ "help", no_argument, NULL, 'h' }, 
		{ "visual", no_argument, NULL, 'v' },
		{ "manual", no_argument, NULL, 'm' },
		
		{ NULL, 0, NULL, 0 }

	};

	/* v^v^v */
	char code[8192]; code[0] = '\0';
	bool visual = false;
	int sleep_amount = 0;
	
	/* process arguments */
	int opt;
	while ((opt = getopt_long(argc, argv, "hmvc:s:", long_options, NULL)) != -1) {
		switch (opt) {

			case 'c':
				strcpy(code, optarg);
				break;
			
			case 's':
				sleep_amount = atoi(optarg);
				if (sleep_amount < 0) {
					sleep_amount = 0;
				}
				break;

			case 'h':
				help(long_options);
				exit(EXIT_SUCCESS);

			case 'v':
				visual = true;
				break;

			case 'm':
				manual();
				exit(EXIT_SUCCESS);
			
			default:
				help(long_options);
				exit(EXIT_FAILURE);
		
		}
	}

	if (code[0] == '\0') {
		printf("No code provided.\n");
		help(long_options);
		exit(EXIT_FAILURE);
	}

	/* blocks -> errors -> interpret */
	Block *blocks = to_blocks(code);
	char *errors = block_errors(blocks, BUFFER_SIZE);
	if (strlen(errors) != 0) {
		for (int i = 0; i < BUFFER_SIZE; i++) {
			if (errors[i] == '\0') {
				break;
			}
			printf("invalid character '%c'\n", errors[i]);
		}
		exit(EXIT_FAILURE);	
	}

	// Run the code
	if (visual) {

    initscr();
    cbreak(); 
    keypad(stdscr, TRUE); 
    noecho();

		interpret(blocks, visual, sleep_amount);
    
    endwin();

	} else {

		interpret(blocks, visual, sleep_amount);
	
	}

	// Free allocated memory
	free_blocks(blocks);
	free(errors);
	free(g_buffer);
	for (int i = 0; i < g_funp; i++) {
		free(g_functions[i]);
	}
	free(g_functions);

  return 0;
}

void interpret(Block *blocks, bool visual, int wait) {

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
        interpret((Block *)current_block->value, visual, wait);
      }

      break;
    }

    case ACT_AUTO_LOOP: {

      char *iter_count = &(g_buffer[g_bufp]);
      while (*iter_count > 0) {
        interpret((Block *)current_block->value, visual, wait);
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

      interpret(g_functions[(int)(g_buffer[g_bufp])], visual, wait);

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
		
		if (visual) {
			draw_gbuffer(current_block);
		}

		sleep(wait);
  }
}

void help(struct option opts[]) {
	printf("\n");
	printf("Parameters: \n");
	while (opts->name != NULL) {
		printf(" --%s/-%c", opts->name, opts->val);
		switch (opts->has_arg) {

			case required_argument:
				printf(" <arg>");
				break;

			case optional_argument:
				printf(" [arg]");
				break;

			default:
				break;

		}
		printf("\n");
		opts++;
	}
	printf("\n");
}

void manual() {
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
}

void draw_gbuffer(Block* blk) {
	clear();
	int w, h;
	int x = 0, y = 0;
	getmaxyx(stdscr, w, h);
	for (int i = 0; i < BUFFER_SIZE; i++) {
		char buf[5]; buf[3] = ' '; buf[4] = '\0';
		for (int j = 0; j < 4; j++) {
			buf[j] = ' ';
		}
		sprintf(buf, "%d", (int)(g_buffer[i]));
		mvprintw(x+1, y, "%s", buf);
		y++;
		if (y > w) {
			x++;
			y = 0;
		}
		if (x > h) {
			break;
		}
	}

	mvprintw(0, 0, "%s", action_str(blk->action)); 
	refresh();
}

char* read_file(char* fn) {
	FILE* file = fopen(fn, "r");
	if (file == NULL) {
		return NULL;
	}

	char* buffer; long fs;
	fseek(file, 0, SEEK_END);
	fs = ftell(file);
	rewind(file);

	buffer = (char*)malloc(fs+1);
	if (buffer == NULL) {
		return NULL;
	}

	if (fread(buffer, 1, fs, file) != fs) {
		free(buffer); fclose(file); return NULL;
	}

	buffer[fs] = '\0'; // valid c string :)
	
	fclose(file);

	return buffer;
}

