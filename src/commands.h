#ifndef COMMANDS_H

#define COMMANDS_H

#define PARAMS_LIST_SIZE 30
#define PARAM_LEN 50
#define COMMAND_SIZE PARAM_LEN + PARAM_LEN *PARAMS_LIST_SIZE

typedef struct {
  char *executable;
  char **argv;
} command_t;

int handle_shell_commands(char *command_buf);
int exec_command(command_t *command, int *fds);
void free_cmd(command_t *cmd);
void command_to_str(char buf[COMMAND_SIZE], command_t *cmd);

#endif
