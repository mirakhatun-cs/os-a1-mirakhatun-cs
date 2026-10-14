
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// Match filenames with patterns containing *
int
match(char *pattern, char *name)
{
  if (*pattern == '\0')
    return *name == '\0';

  if (*pattern == '*') {
    // Match zero characters or consume one character
    return match(pattern + 1, name) ||
           (*name != '\0' && match(pattern, name + 1));
  }

  if (*name != '\0' && *pattern == *name)
    return match(pattern + 1, name + 1);

  return 0;
}

// Search a directory and all its subdirectories
void
find(char *path, char *pattern, int type)
{
  int fd;
  struct dirent de;
  struct stat st;
  char buf[512];

  fd = open(path, O_RDONLY);
  if (fd < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    close(fd);
    return;
  }

  if (st.type != T_DIR) {
    close(fd);
    return;
  }

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;

    char name[DIRSIZ + 1];
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = '\0';

    if (strcmp(name, ".") == 0 ||
        strcmp(name, "..") == 0)
      continue;

    if (strlen(path) + 1 + strlen(name) + 1 > sizeof(buf))
      continue;

    strcpy(buf, path);
    char *p = buf + strlen(buf);
    *p++ = '/';
    strcpy(p, name);

    if (stat(buf, &st) < 0)
      continue;

    int type_ok = type == 0 ||
                  (type == 1 &&
                   (st.type == T_FILE || st.type == T_DEVICE)) ||
                  (type == 2 && st.type == T_DIR);

    if (type_ok && match(pattern, name))
      printf("%s\n", buf);

    if (st.type == T_DIR)
      find(buf, pattern, type);
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  int type = 0;

  if (argc != 3 && argc != 5) {
    fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
    exit(1);
  }

  if (argc == 5) {
    if (strcmp(argv[3], "-type") != 0) {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }

    if (strcmp(argv[4], "f") == 0)
      type = 1;
    else if (strcmp(argv[4], "d") == 0)
      type = 2;
    else {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }
  }

  // Validate the starting directory
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }
  close(fd);

  find(argv[1], argv[2], type);
  exit(0);
}
