
#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(void)
{
  struct sysinfo info;

  if (sysinfo(&info) < 0) {
    fprintf(2, "sysinfo: failed\n");
    exit(1);
  }

  printf("Free memory: %lu bytes\n", info.freemem);
  printf("Processes: %lu\n", info.nproc);
  printf("Open files: %lu\n", info.nopenfile);

  // Check an invalid user-space pointer.
  if (sysinfo((struct sysinfo *)-1) != -1) {
    fprintf(2, "sysinfo: invalid pointer test failed\n");
    exit(1);
  }

  printf("sysinfo basic tests passed\n");
  exit(0);
}
