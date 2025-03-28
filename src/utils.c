#include "main.h"
#include <stdio.h>
#include <string.h>

void type_prompt() { printf("Type your command: "); }

void build_full_command(char buf[COMMAND_SIZE], char *command_buf,
                        char **params_buf) {
  strlcpy(buf, command_buf, COMMAND_SIZE);
  strlcat(buf, " ", COMMAND_SIZE);
  for (int i = 1; params_buf[i] != NULL && i < PARAMS_LIST_SIZE; i++)
    strlcat(buf, params_buf[i], COMMAND_SIZE);
}
