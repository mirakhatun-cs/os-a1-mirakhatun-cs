
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXWORDS 512
#define INPUTSIZE 4096
static char input[INPUTSIZE];
static char *words[MAXWORDS];

void
usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

// Run one command and wait until it finishes.
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
  int limit = MAXARG;
  int use_n = 0;
  int base = 0;
  int failed = 0;

  if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
    if (argc < 4)
      usage();

    // Validate N: only positive decimal integers.
    for (int i = 0; argv[2][i]; i++) {
      if (argv[2][i] < '0' || argv[2][i] > '9')
        usage();
    }

    if (argv[2][0] == '\0')
      usage();

    limit = atoi(argv[2]);
    if (limit < 1)
      usage();

    use_n = 1;
    start = 3;
  }

  if (start >= argc)
    usage();

  char *cmd[MAXARG + 1];

  for (int i = start; i < argc; i++) {
    if (base >= MAXARG)
      usage();
    cmd[base++] = argv[i];
  }

  int capacity = MAXARG - base;

  if (capacity < 1)
    usage();

  if (limit > capacity)
    limit = capacity;

  
  int nwords = 0;
  int pos = 0;
  int inword = 0;

  // Read all stdin, preserving line boundaries.
  char c;
  int n;

  while ((n = read(0, &c, 1)) == 1) {
    if (pos >= INPUTSIZE - 1) {
      fprintf(2, "xargs: input too long\n");
      exit(1);
    }

    if (c == ' ' || c == '\t' || c == '\n') {
      if (inword) {
        input[pos++] = '\0';
        inword = 0;
      }

      if (c == '\n' && !use_n) {
        if (nwords > 0) {
          int offset = 0;

          while (offset < nwords) {
            int count = nwords - offset;
            if (count > capacity)
              count = capacity;

            for (int j = 0; j < count; j++)
              cmd[base + j] = words[offset + j];

            cmd[base + count] = 0;
            if (run(cmd))
              failed = 1;

            offset += count;
          }
        }
        nwords = 0;
      }
    } else {
      if (!inword) {
        if (nwords >= MAXWORDS) {
          fprintf(2, "xargs: too many words\n");
          exit(1);
        }
        words[nwords++] = &input[pos];
        inword = 1;
      }
      input[pos++] = c;
    }

    if (use_n && !inword && nwords >= limit) {
      for (int j = 0; j < nwords; j++)
        cmd[base + j] = words[j];

      cmd[base + nwords] = 0;
      if (run(cmd))
        failed = 1;

      nwords = 0;
      pos = 0;
    }

    if (!use_n && c == '\n')
      pos = 0;
  }

  if (n < 0) {
    fprintf(2, "xargs: read failed\n");
    exit(1);
  }

  if (inword)
    input[pos++] = '\0';

  if (nwords > 0) {
    int offset = 0;

    while (offset < nwords) {
      int count = nwords - offset;
      if (count > (use_n ? limit : capacity))
        count = use_n ? limit : capacity;

      for (int j = 0; j < count; j++)
        cmd[base + j] = words[offset + j];

      cmd[base + count] = 0;
      if (run(cmd))
        failed = 1;

      offset += count;
    }
  }

  exit(failed ? 1 : 0);
}
