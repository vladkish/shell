#include "history.h"
#include "../main.h"
#include "../utils.h"
#include <stdio.h>

static char commands_history[HISTORY_LEN][COMMAND_SIZE];
static int history_pos;

void add_to_history(char *command_buf, char **params_buf) {
  build_full_command(commands_history[history_pos], command_buf, params_buf);
  history_pos++;
}

void show_history() {
  if (history_pos == 0) {
    printf("Commands history is empty!\n");
    return;
  }
  for (int i = 0; i < history_pos; i++) {
    printf("%s\n", commands_history[i]);
  }
}
