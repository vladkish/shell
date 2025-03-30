
#ifndef UTILS_H

#define UTILS_H
#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define is_parent_proc(pid) (pid != 0)

void type_prompt();

#endif
