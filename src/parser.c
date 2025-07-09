#include "commands.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SHOW_LAST_CMD_STATUS "_lcs"
#define SHOW_CURRENT_PID "§§"

int parse_command(command_t *cmd) {
  char ch;
  int i = 0;
  for (; (ch = getchar()) != ' ' && ch != '\n' && i < PARAM_LEN; i++) {
    cmd->executable[i] = ch;
  }
  cmd->executable[i] = 0;
  if (!strlen(cmd->executable))
    return -1;
  strncpy(cmd->argv[0], cmd->executable, PARAM_LEN);
  i = 1;
  // scanning parameters of the command
  for (; ch != '\n' && i < PARAMS_LIST_SIZE - 1; i++) {
    while ((ch = getchar()) == ' ') // skip all spaces
      ;
    int j = 0;
    for (; ch != ' ' && ch != '\n' && j < PARAM_LEN - 1; j++) {
      cmd->argv[i][j] = ch;
      ch = getchar();
    };
    cmd->argv[i][j] = 0;
  }
  cmd->argv[i] = NULL;
  return i;
}

static void params_shift(char *params[], int shift_from) {
  int i = shift_from;
  for (; params[i + 1] != NULL; i++) {
    params[i] = params[i + 1];
  }
  params[i] = NULL;
}

static int is_valid_redirect_param(char *params[], int i, char *comparable) {
  return strcmp(params[i], comparable) == 0 && params[i + 1] != NULL;
}

static int get_redirect_fd(char *params[], int i, int flags) {
  int fd;
  if ((fd = open(params[i + 1], flags)) == -1) {
    return -1;
  }
  params_shift(params, i);
  params_shift(params, i);
  return fd;
}

// If invalid params supplied - returns NULL. Otherwise returns array with 3
// elements length of opened file descriptors, where idx corresponds to std
// stream for redirect from, and value corresponds to a new file descriptor
// Also that function removes found redirect params by shifting params array
int *extract_redirects(char *params[]) {
  int *fds_for_dup = malloc(3 * sizeof(int));
  int fd;
  for (int i = 0; i < 3; i++)
    fds_for_dup[i] = -1;
  for (int i = 1; params[i] != NULL; i++) {
    printf("Checking: %s|\n", params[i]);
    if (is_valid_redirect_param(params, i, ">")) {
      if ((fd = get_redirect_fd(params, i, O_CREAT | O_WRONLY)) == -1) {
        return NULL;
      }
      fds_for_dup[1] = fd;
      /* printf("Shift completed\n"); */
      /* for (int i = 0; params[i] != NULL; i++) { */
      /*   printf("param: %s\n", params[i]); */
      /* } */
    } else if (is_valid_redirect_param(params, i, "<")) {
      if ((fd = get_redirect_fd(params, i, O_CREAT | O_RDONLY)) == -1) {
        return NULL;
      }
      fds_for_dup[0] = fd;
    } else if (is_valid_redirect_param(params, i, ">>")) {
      if ((fd = get_redirect_fd(params, i, O_CREAT | O_WRONLY | O_APPEND)) ==
          -1) { 
        return NULL;
      }
      fds_for_dup[1] = fd;
    }
  }
  return fds_for_dup;
}
