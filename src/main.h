#ifndef MAIN_H

#define MAIN_H

#define RUN_BACKGROUND '&'

typedef struct {
  void *stdin;
  void *stdout;
  void *stderr;
} std_streams;

typedef enum { STDIN, STDOUT, STDERR } streams;

typedef struct {
  streams type;
  char *filepath;
} stream_redirect;

#endif
