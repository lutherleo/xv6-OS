#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define BUFSZ 1024

static char buf[BUFSZ];

// Tail code
static void
tailfd(int fd, int nlines)
{
  int i, n;
  int total_lines = 0;
  int seen = 0;

  unlink(".__tailtmp__");

  int tmp = open(".__tailtmp__", O_CREATE | O_RDWR);
  if (tmp < 0) {
    printf(2, "tail: cannot create temporary file\n");
    exit();
  }

  while ((n = read(fd, buf, sizeof(buf))) > 0) {
    write(tmp, buf, n);
    for (i = 0; i < n; i++)
      if (buf[i] == '\n') total_lines++;
  }
  if (n < 0) {
    printf(2, "tail: read error\n");
    close(tmp);
    unlink(".__tailtmp__");
    exit();
  }

  close(tmp);
  tmp = open(".__tailtmp__", 0);
  if (tmp < 0) {
    printf(2, "tail: cannot reopen temporary file\n");
    unlink(".__tailtmp__");
    exit();
  }

  int start_line = total_lines - nlines;
  if (start_line < 0) start_line = 0;

  while ((n = read(tmp, buf, sizeof(buf))) > 0) {
    for (i = 0; i < n; i++) {
      if (seen >= start_line)
        write(1, &buf[i], 1);
      if (buf[i] == '\n')
        seen++;
    }
  }
  if (n < 0)
    printf(2, "tail: read error (second pass)\n");

  close(tmp);
  unlink(".__tailtmp__");
}


// Main file
int
main(int argc, char *argv[])
{
  int nlines = 10;   
  int start = 1;     
  int i, fd;


  if (argc > 1 && argv[1][0] == '-') {
    if (argv[1][1] == 'n') {
      if (argc < 3) {
        printf(2, "usage: tail -n N [FILE...]\n");
        exit();
      }
      nlines = atoi(argv[2]);
      start = 3;
    } else {

      nlines = atoi(argv[1] + 1);
      start = 2;
    }
    if (nlines <= 0) {
      printf(2, "tail: invalid line count\n");
      exit();
    }
  }


  if (start >= argc) {
    tailfd(0, nlines);
    exit();
  }


  for (i = start; i < argc; i++) {
    fd = open(argv[i], 0);
    if (fd < 0) {
      printf(2, "tail: cannot open %s\n", argv[i]);
      continue;
    }
    tailfd(fd, nlines);
    close(fd);
  }

  exit();
}
