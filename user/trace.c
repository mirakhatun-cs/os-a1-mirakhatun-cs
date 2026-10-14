
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
    exit(1);
  }

  // Validate the tracing mask.
  for (int i = 0; argv[1][i] != '\0'; i++) {
    if (argv[1][i] < '0' || argv[1][i] > '9') {
      fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
      exit(1);
    }
  }

  if (argv[1][0] == '\0') {
    fprintf(2, "usage: trace <mask> <cmd> [args...]\n");
    exit(1);
  }

  int mask = atoi(argv[1]);

  // Enable tracing for this process.
  if (trace(mask) < 0) {
    fprintf(2, "trace: system call failed\n");
    exit(1);
  }

  // Run the requested command.
  exec(argv[2], &argv[2]);

  // Reached only if exec fails.
  fprintf(2, "trace: exec failed\n");
  exit(1);
}
