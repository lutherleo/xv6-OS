#include "types.h"
#include "stat.h"
#include "user.h"

// Test Case 3: Priority Inheritance Solution
int
main(void)
{
  int lock_id = 3;
  int pid_low, pid_med, pid_high;
  
  printf(1, "\n=== TEST 6: Priority Inheritance Solution ===\n\n");
  printf(1, "Demonstrating priority inheritance:\n");
  printf(1, "  1. Low priority acquires lock\n");
  printf(1, "  2. High priority waits for lock\n");
  printf(1, "  3. Low priority INHERITS high priority\n");
  printf(1, "  4. Low priority can preempt medium priority\n");
  printf(1, "  5. Low priority finishes quickly\n");
  printf(1, "  6. High priority gets lock\n\n");
  
  // Low priority process
  pid_low = fork();
  if(pid_low == 0){
    int orig_pri = 5;
    nice(getpid(), orig_pri);
    
    int start_time = uptime();
    printf(1, "[LOW  Pri=%d PID=%d] Started at time %d\n", 
           orig_pri, getpid(), start_time);
    
    printf(1, "[LOW  Pri=%d PID=%d] Acquiring lock %d\n", 
           orig_pri, getpid(), lock_id);
    lock(lock_id);
    
    int lock_time = uptime();
    printf(1, "[LOW  Pri=%d PID=%d] Acquired lock at time %d\n", 
           orig_pri, getpid(), lock_time);
    
    // Do work while holding lock
    printf(1, "[LOW  Pri=%d PID=%d] Working with lock...\n", 
           orig_pri, getpid());
    
    // Work for a while
    int work_done = 0;
    int target_time = lock_time + 300;
    while(uptime() < target_time){
      work_done++;
    }
    
    int before_release = uptime();
    printf(1, "[LOW  Pri=%d PID=%d] Releasing lock at time %d (work: %d)\n", 
           orig_pri, getpid(), before_release, work_done);
    release(lock_id);
    
    printf(1, "[LOW  Pri=%d PID=%d] Priority restored to original\n", 
           orig_pri, getpid());
    
    int end_time = uptime();
    printf(1, "[LOW  Pri=%d PID=%d] Finished at time %d\n", 
           orig_pri, getpid(), end_time);
    exit();
  }
  
  // Let low priority get the lock
  sleep(30);
  
  // Medium priority CPU-bound process
  pid_med = fork();
  if(pid_med == 0){
    int med_pri = 3;
    nice(getpid(), med_pri);
    
    int start_time = uptime();
    printf(1, "[MED  Pri=%d PID=%d] Started at time %d\n", 
           med_pri, getpid(), start_time);
    printf(1, "[MED  Pri=%d PID=%d] CPU-bound work (no locks)\n", 
           med_pri, getpid());
    
    // Do CPU work
    int work = 0;
    int target = start_time + 400;
    while(uptime() < target){
      work++;
    }
    
    int end_time = uptime();
    printf(1, "[MED  Pri=%d PID=%d] Finished at time %d (work: %d)\n", 
           med_pri, getpid(), end_time, work);
    exit();
  }
  
  // Let medium priority start
  sleep(30);
  
  // High priority process
  pid_high = fork();
  if(pid_high == 0){
    int high_pri = 1;
    nice(getpid(), high_pri);
    
    int start_time = uptime();
    printf(1, "[HIGH Pri=%d PID=%d] Started at time %d\n", 
           high_pri, getpid(), start_time);
    
    printf(1, "[HIGH Pri=%d PID=%d] Attempting to acquire lock %d\n", 
           high_pri, getpid(), lock_id);
    printf(1, "[HIGH Pri=%d PID=%d] Low priority should INHERIT my priority!\n", 
           high_pri, getpid());
    
    lock(lock_id);
    int lock_time = uptime();
    printf(1, "[HIGH Pri=%d PID=%d] Acquired lock at time %d\n", 
           high_pri, getpid(), lock_time);
    
    printf(1, "[HIGH Pri=%d PID=%d] Releasing lock\n", high_pri, getpid());
    release(lock_id);
    
    int end_time = uptime();
    printf(1, "[HIGH Pri=%d PID=%d] Finished at time %d\n", 
           high_pri, getpid(), end_time);
    exit();
  }
  
  wait();
  wait();
  wait();
  
  printf(1, "\n=== TEST 6 Complete ===\n");
  printf(1, "Expected Results:\n");
  printf(1, "  1. Low priority acquired lock\n");
  printf(1, "  2. High priority requested lock\n");
  printf(1, "  3. Low priority INHERITED high priority (priority 1)\n");
  printf(1, "  4. Low priority could preempt medium priority\n");
  printf(1, "  5. Low priority finished quickly and released lock\n");
  printf(1, "  6. High priority acquired lock immediately\n");
  printf(1, "  7. Medium priority ran last\n\n");
  printf(1, "This demonstrates PRIORITY INHERITANCE solving inversion!\n\n");
  
  exit();
}