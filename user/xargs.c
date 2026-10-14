
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define INPUTSIZE 4096

static char input[INPUTSIZE];
static char *cmd[MAXARG + 1];

void
usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

int
run(char *argv[])
{
  int pid = fork();
  int status;

  if (pid < 0)
    return 1;

  if (pid == 0) {
    exec(argv[0], argv);
    fprintf(2, "xargs: exec failed\n");
    exit(1);
  }

  if (wait(&status) < 0)
    return 1;

  return status != 0;
}

int
main(int argc, char *argv[])
{
  int start = 1;
  int use_n = 0;
  int limit = 0;
  int base = 0;
  int failed = 0;
  int pos = 0;
  int nwords = 0;
  int inword = 0;
  char c;
  int n;

  if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
    if (argc < 4 || argv[2][0] == '\0')
      usage();

    int value = 0;

    for (int i = 0; argv[2][i]; i++) {
      char ch = argv[2][i];

      if (ch < '0' || ch > '9')
        usage();

      int digit = ch - '0';

      if (value > (MAXARG - digit) / 10)
        usage();

      value = value * 10 + digit;
    }

    if (value < 1)
      usage();

    use_n = 1;
    limit = value;
    start = 3;
  }

  if (start >= argc)
    usage();

  for (int i = start; i < argc; i++) {
    if (base >= MAXARG)
      usage();
    cmd[base++] = argv[i];
  }

  int capacity = MAXARG - base;

  if (capacity < 1)
    usage();

  if (!use_n)
    limit = capacity;

  if (limit > capacity)
    limit = capacity;

  cmd[base] = 0;

  while ((n = read(0, &c, 1)) == 1) {

    int separator = (c == ' ' || c == '\t' ||
                     c == '\n');

    if (separator) {

      if (inword) {
        input[pos++] = '\0';
        inword = 0;
      }

      if (use_n && nwords == limit) {
        cmd[base + nwords] = 0;

        if (run(cmd))
          failed = 1;

        nwords = 0;
        pos = 0;
      }

      if (!use_n && c == '\n') {
        if (nwords > 0) {
          cmd[base + nwords] = 0;

          if (run(cmd))
            failed = 1;
        }

        nwords = 0;
        pos = 0;
      }

    } else {

      if (!inword) {
        if (nwords >= limit) {
          if (!use_n) {
            fprintf(2, "xargs: too many arguments\n");
            exit(1);
          }

          cmd[base + nwords] = 0;

          if (run(cmd))
            failed = 1;

          nwords = 0;
          pos = 0;
        }

        cmd[base + nwords] = &input[pos];
        nwords++;
        inword = 1;
      }

      if (pos >= INPUTSIZE - 1) {
        fprintf(2, "xargs: input too long\n");
        exit(1);
      }

      input[pos++] = c;
    }
  }

  if (n < 0) {
    fprintf(2, "xargs: read failed\n");
    exit(1);
  }

  if (inword)
    input[pos++] = '\0';

  if (nwords > 0) {
    cmd[base + nwords] = 0;

    if (run(cmd))
      failed = 1;
  }

  exit(failed ? 1 : 0);
}
