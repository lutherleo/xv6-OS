#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int pid, value, oldpri;
  
  if(argc < 2 || argc > 3) {
    printf(2, "Usage: nice [pid] value\n");
    printf(2, "  value must be between 1 and 5\n");
    exit();
  }
  
  if(argc == 2) {
    value = atoi(argv[1]);
    pid = getpid();
  } else {
    pid = atoi(argv[1]);
    value = atoi(argv[2]);
  }
  
  oldpri = nice(pid, value);
  
  if(oldpri < 0) {
    printf(2, "nice: failed to set priority\n");
    exit();
  }
  
  printf(1, "%d %d\n", pid, oldpri);
  exit();
}