#include "types.h"
#include "stat.h"
#include "user.h"

void
work_and_report(int id, int priority)
{
  int start = uptime();
  int count = 0;
  
  printf(1, "[Process %d] Priority %d starting at time %d\n", id, priority, start);
  
  // Do some work
  for(int i = 0; i < 30000; i++){
    for(int j = 0; j < 100; j++){
      count += (i * j) % 100;
    }
  }
  
  int end = uptime();
  int turnaround = end - start;
  
  printf(1, "[Process %d] Priority %d finished at time %d (turnaround: %d ticks)\n", 
         id, priority, end, turnaround);
  
  exit();
}

int
main(void)
{
  int pid1, pid2, pid3, pid4, pid5;
  
  printf(1, "\n=== TEST 3: Turnaround Time Test ===\n");
  printf(1, "Five processes with all priority levels (1-5)\n");
  printf(1, "Lower turnaround time = got more CPU time\n\n");
  
  // Create all 5 priority levels
  pid1 = fork();
  if(pid1 == 0){
    nice(getpid(), 1);
    work_and_report(1, 1);
  }
  
  pid2 = fork();
  if(pid2 == 0){
    nice(getpid(), 2);
    work_and_report(2, 2);
  }
  
  pid3 = fork();
  if(pid3 == 0){
    nice(getpid(), 3);
    work_and_report(3, 3);
  }
  
  pid4 = fork();
  if(pid4 == 0){
    nice(getpid(), 4);
    work_and_report(4, 4);
  }
  
  pid5 = fork();
  if(pid5 == 0){
    nice(getpid(), 5);
    work_and_report(5, 5);
  }
  
  // Wait for all
  wait();
  wait();
  wait();
  wait();
  wait();
  
  printf(1, "\n=== TEST 3 Complete ===\n");
  printf(1, "Expected: Priority 1 has lowest turnaround time\n");
  printf(1, "          Priority 5 has highest turnaround time\n");
  printf(1, "          Turnaround time increases with priority number\n\n");
  
  exit();
}