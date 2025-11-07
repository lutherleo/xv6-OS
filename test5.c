#include "types.h"
#include "stat.h"
#include "user.h"

// Test Case 2: Priority Inversion Demonstration
int
main(void)
{
  int lock_id = 2;
  int pid_low, pid_high;
  
  printf(1, "\n=== TEST 5: Priority Inversion Demo ===\n\n");
  printf(1, "Demonstrating the priority inversion problem:\n");
  printf(1, "  Low priority process acquires lock\n");
  printf(1, "  High priority process tries to acquire same lock\n");
  printf(1, "  High priority must wait for low priority\n\n");
  
  // Low priority process
  pid_low = fork();
  if(pid_low == 0){
    nice(getpid(), 5);  // Set to lowest priority
    
    int start_time = uptime();
    printf(1, "[LOW  Priority PID=%d] Started at time %d\n", getpid(), start_time);
    
    printf(1, "[LOW  Priority PID=%d] Acquiring lock %d\n", getpid(), lock_id);
    lock(lock_id);
    int lock_time = uptime();
    printf(1, "[LOW  Priority PID=%d] Acquired lock %d at time %d\n", 
           getpid(), lock_id, lock_time);
    
    // Hold lock and do work for ~5 seconds worth of ticks
    printf(1, "[LOW  Priority PID=%d] Holding lock and working for 500 ticks...\n", getpid());
    int target_time = lock_time + 500;
    while(uptime() < target_time){
      // Simulate work
      for(int i = 0; i < 100000; i++);
    }
    
    int release_time = uptime();
    printf(1, "[LOW  Priority PID=%d] Releasing lock %d at time %d\n", 
           getpid(), lock_id, release_time);
    release(lock_id);
    
    int end_time = uptime();
    printf(1, "[LOW  Priority PID=%d] Finished at time %d (total: %d ticks)\n", 
           getpid(), end_time, end_time - start_time);
    exit();
  }
  
  // Wait a bit for low priority to acquire lock
  sleep(50);
  
  // High priority process
  pid_high = fork();
  if(pid_high == 0){
    nice(getpid(), 1);  // Set to highest priority
    
    int start_time = uptime();
    printf(1, "[HIGH Priority PID=%d] Started at time %d\n", getpid(), start_time);
    
    printf(1, "[HIGH Priority PID=%d] Attempting to acquire lock %d\n", getpid(), lock_id);
    printf(1, "[HIGH Priority PID=%d] WAITING for low priority process...\n", getpid());
    
    lock(lock_id);
    int lock_time = uptime();
    printf(1, "[HIGH Priority PID=%d] Acquired lock %d at time %d (waited %d ticks)\n", 
           getpid(), lock_id, lock_time, lock_time - start_time);
    
    printf(1, "[HIGH Priority PID=%d] Releasing lock %d\n", getpid(), lock_id);
    release(lock_id);
    
    int end_time = uptime();
    printf(1, "[HIGH Priority PID=%d] Finished at time %d (total: %d ticks)\n", 
           getpid(), end_time, end_time - start_time);
    exit();
  }
  
  wait();
  wait();
  
  printf(1, "\n=== TEST 5 Complete ===\n");
  printf(1, "Observations:\n");
  printf(1, "  - Low priority acquired lock first\n");
  printf(1, "  - High priority had to WAIT for low priority\n");
  printf(1, "  - This demonstrates PRIORITY INVERSION problem\n");
  printf(1, "  - High priority process was blocked by low priority!\n\n");
  
  exit();
}