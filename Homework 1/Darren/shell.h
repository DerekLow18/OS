/*
  header file for shell.  Adapted from Operating Systems, Nutt.
*/
#define DEBUG
#define TRUE 1
#define FALSE 0
#define LINE_LEN 80

#define MAX_ARGS 64
#define MAX_ARG_LEN 16

#define MAX_PATHS 3
#define MAX_PATH_LEN 96

#define WHITESPACE " .,\t\n"
#define DELIM ':'

#define PROMPT "$: "

/* Store commands in this structure */
struct Command {
  int argc; // number of args
  char *argv[MAX_ARGS]; // args.  arg[0] is command
};
