#include "commands.h"
#include "features/bg_jobs.h"
#include "features/history.h"
#include "utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define RUN_BACKGROUND '&'
#define INITIAL_CMD_CAPACITY 10

typedef struct {
    char* executable;
    char** argv;
    size_t argc;
    size_t capacity;
} command_t;

int parse_command(command_t *cmd);
int* extract_redirects(char *params[]);
void init_command(command_t *cmd);
void free_command(command_t *cmd);
void resize_command(command_t *cmd);

int main() {
    int statloc, last_cmd_status = 0;
    pthread_t thread;
    
    while (1) {
        type_prompt();
        
        command_t cmd;
        init_command(&cmd);
        
        if (parse_command(&cmd) == -1) {
            free_command(&cmd);
            continue;
        }
        
        if (handle_shell_commands(cmd.executable)) {
            free_command(&cmd);
            continue;
        }
        
        add_to_history(&cmd);
        
        int *fds_for_dup = extract_redirects(cmd.argv);
        if (fds_for_dup == NULL) {
            perror("open");
            free_command(&cmd);
            continue;
        }
        
        if (cmd.argc > 0 && *cmd.argv[cmd.argc - 1] == RUN_BACKGROUND) {
            cmd.argv[cmd.argc - 1] = NULL;
            cmd.argc--;
            
            struct job_params job_params = {
                .command = cmd,
                .fds_to_dup = fds_for_dup
            };
            
            if (pthread_create(&thread, NULL, run_bg_job, &job_params) != 0) {
                handle_error("Failed to create thread");
            };
        } else {
            pid_t runner_pid = exec_command(&cmd, fds_for_dup);
            if (runner_pid == -1) {
                printf("Failed to launch command!\n");
            } else {
                waitpid(runner_pid, &statloc, 0);
                last_cmd_status = WEXITSTATUS(statloc);
                if (last_cmd_status != EXIT_SUCCESS) {
                    printf("Command exited with status %d\n", last_cmd_status);
                }
            }
        }
        
        free_command(&cmd);
        free(fds_for_dup);
    }
    return 0;
}

void init_command(command_t *cmd) {
    cmd->executable = malloc(PARAM_LEN);
    cmd->argv = malloc(sizeof(char*) * INITIAL_CMD_CAPACITY);
    cmd->argc = 0;
    cmd->capacity = INITIAL_CMD_CAPACITY;
    for (size_t i = 0; i < cmd->capacity; i++) {
        cmd->argv[i] = malloc(PARAM_LEN);
    }
}

void free_command(command_t *cmd) {
    free(cmd->executable);
    for (size_t i = 0; i < cmd->capacity; i++) {
        free(cmd->argv[i]);
    }
    free(cmd->argv);
}

void resize_command(command_t *cmd) {
    size_t new_capacity = cmd->capacity * 2;
    cmd->argv = realloc(cmd->argv, sizeof(char*) * new_capacity);
    for (size_t i = cmd->capacity; i < new_capacity; i++) {
        cmd->argv[i] = malloc(PARAM_LEN);
    }
    cmd->capacity = new_capacity;
}