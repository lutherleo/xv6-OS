#include "types.h"
#include "stat.h"
#include "user.h"

// CPU-bound process
void
cpu_work(int id)
{
  int count = 0;
  
  printf(1, "[CPU Process %d] Priority 1 - Started\n", id);
  
  // Pure CPU work
  for(int i = 0; i < 50000; i++){
    for(int j = 0; j < 100; j++){
      count++;
    }
  }
  
  printf(1, "[CPU Process %d] Priority 1 - Finished with count=%d\n", id, count);
  exit();
}

// I/O-bound process (sleeps frequently)
void
io_work(int id)
{
  int count = 0;
  
  printf(1, "[I/O Process %d] Priority 5 - Started\n", id);
  
  // Mix of work and I/O
  for(int i = 0; i < 100; i++){
    for(int j = 0; j < 500; j++){
      count++;
    }
    sleep(1);  // Simulate I/O
  }
  
  printf(1, "[I/O Process %d] Priority 5 - Finished with count=%d\n", id, count);
  exit();
}

int
main(void)
{
  int pid1, pid2;
  
  printf(1, "\n=== TEST 2: I/O vs CPU Bound ===\n");
  printf(1, "CPU process (priority 1) vs I/O process (priority 5)\n\n");
  
  // Start I/O bound process first
  pid1 = fork();
  if(pid1 == 0){
    nice(getpid(), 5);  // Low priority
    io_work(1);
  }
  
  // Start CPU bound process
  pid2 = fork();
  if(pid2 == 0){
    nice(getpid(), 1);  // High priority
    cpu_work(2);
  }
  
  wait();
  wait();
  
  printf(1, "\n=== TEST 2 Complete ===\n");
  printf(1, "Expected: CPU process (priority 1) should run when I/O process sleeps\n");
  printf(1, "          CPU process completes faster despite I/O having lower priority\n\n");
  
  exit();
}