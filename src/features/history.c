#include "history.h"
#include <stdio.h>

static char commands_history[HISTORY_LEN][COMMAND_SIZE];
static int history_pos;

void add_to_history(command_t *cmd) {
  command_to_str(commands_history[history_pos], cmd);
  history_pos++;
}

void show_history() {
  if (history_pos == 0) {
    printf("Commands history is empty!\n");
    return;
  }
  for (int i = 0; i < history_pos; i++) {
    printf("%d. %s\n", i + 1, commands_history[i]);
  }
}
