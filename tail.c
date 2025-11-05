// tail.c  (xv6 user program)
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define TAIL_TMP "._tail_tmp_"
#define BUFSZ 1024

static char buf[BUFSZ];

static void
die(const char *msg)
{
  printf(2, "%s\n", msg);
  unlink(TAIL_TMP);
  exit();
}

static void
tail_stream(int fd, int nlines)
{
  int i, n, total_lines = 0;
  int saw_any = 0;
  int last_was_nl = 1;   // treat empty input as 0 lines

  // remove stale file, then create temp
  unlink(TAIL_TMP);
  int tmp = open(TAIL_TMP, O_CREATE | O_RDWR);
  if (tmp < 0) die("tail: cannot create temporary file");

  // pass 1: copy -> tmp, count newlines
  while ((n = read(fd, buf, sizeof(buf))) > 0) {
    if (write(tmp, buf, n) != n) die("tail: write error");
    saw_any = 1;
    for (i = 0; i < n; i++) {
      if (buf[i] == '\n') {
        total_lines++;
        last_was_nl = 1;
      } else {
        last_was_nl = 0;
      }
    }
  }
  if (n < 0) die("tail: read error");

  // count the trailing partial line (no newline at EOF)
  if (saw_any && !last_was_nl)
    total_lines++;

  // nothing requested
  if (nlines <= 0) {
    close(tmp);
    unlink(TAIL_TMP);
    return;
  }

  // pass 2: reopen tmp and print last nlines (or all if fewer exist)
  close(tmp);
  tmp = open(TAIL_TMP, 0);
  if (tmp < 0) die("tail: cannot reopen temporary file");

  int start_line = total_lines - nlines;
  if (start_line < 0) start_line = 0;

  int seen_lines = 0;
  while ((n = read(tmp, buf, sizeof(buf))) > 0) {
    for (i = 0; i < n; i++) {
      // print once we've reached the first byte of the start_line-th line
      if (seen_lines >= start_line)
        printf(1, "%c", buf[i]);
      if (buf[i] == '\n')
        seen_lines++;
    }
  }
  if (n < 0) printf(2, "tail: read error (second pass)\n");

  close(tmp);
  unlink(TAIL_TMP);
}

int
main(int argc, char *argv[])
{
  int nlines = 10;  // default
  int i, idx = 1;

  // parse -n, -nN, -N
  if (argc > 1) {
    if (strcmp(argv[1], "-n") == 0) {
      if (argc < 3) {
        printf(2, "usage:\n  tail [FILE...]\n  tail -N [FILE...]\n  tail -n N [FILE...]\n");
        exit();
      }
      nlines = atoi(argv[2]);
      idx = 3;
    } else if (argv[1][0] == '-' && argv[1][1]) {
      if (argv[1][1] == 'n' && argv[1][2]) {
        // -nN (attached)
        nlines = atoi(argv[1] + 2);
      } else {
        // -N
        nlines = atoi(argv[1] + 1);
      }
      idx = 2;
    }
  }

  // no files → stdin
  if (idx >= argc) {
    tail_stream(0, nlines);
    exit();
  }

  // one or more files; treat "-" as stdin
  for (i = idx; i < argc; i++) {
    int fd;
    if (strcmp(argv[i], "-") == 0) {
      fd = 0;
    } else {
      fd = open(argv[i], 0);
      if (fd < 0) {
        printf(2, "tail: cannot open %s\n", argv[i]);
        continue;
      }
    }
    tail_stream(fd, nlines);
    if (fd != 0) close(fd);
  }

  exit();
}