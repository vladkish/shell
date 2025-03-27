#include "main.h"
#include <stdio.h>
#include <string.h>

int parse_command(char *command_buf, char **params_buf) {
  char ch;
  int i = 0;
  for (; (ch = getchar()) != ' ' && ch != '\n' && i < PARAM_LEN; i++) {
    command_buf[i] = ch;
  }
  command_buf[i] = 0;
  if (!strlen(command_buf))
    return -1;
  params_buf[0] = command_buf;
  i = 1;
  // scanning parameters of the command
  for (; ch != '\n' && i < PARAMS_LIST_SIZE - 1; i++) {
    while ((ch = getchar()) == ' ') // skip all spaces
      ;
    int j = 0;
    for (; ch != ' ' && ch != '\n' && j < PARAM_LEN - 1; j++) {
      params_buf[i][j] = ch;
      ch = getchar();
    };
    params_buf[i][j] = 0;
  }
  params_buf[i] = NULL;
  return i;
}

void is_shell_param(char *params[]) {
  int i = 0;
  while (params[i] != NULL) {
    if (strncmp(params[i], REDIRECT_STDOUT_APPEND, 2) &&
        params[i + 1] != NULL) {
    }
  }
}
