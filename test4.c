#include "types.h"
#include "stat.h"
#include "user.h"

// Test Case 1: Basic Lock/Unlock
int
main(void)
{
  int lock_id = 1;
  int pid1, pid2;
  
  printf(1, "\n=== TEST 4: Basic Lock/Unlock ===\n\n");
  
  // Test 1: Invalid lock IDs
  printf(1, "Test 1a: Testing invalid lock ID (-1)\n");
  if(lock(-1) < 0){
    printf(1, "  PASS: Rejected invalid lock ID -1\n");
  } else {
    printf(1, "  FAIL: Accepted invalid lock ID\n");
  }
  
  printf(1, "Test 1b: Testing invalid lock ID (8)\n");
  if(lock(8) < 0){
    printf(1, "  PASS: Rejected invalid lock ID 8\n");
  } else {
    printf(1, "  FAIL: Accepted invalid lock ID\n");
  }
  
  // Test 2: Valid lock acquire and release
  printf(1, "\nTest 2: Acquire and release lock %d\n", lock_id);
  if(lock(lock_id) == 0){
    printf(1, "  PASS: Acquired lock %d\n", lock_id);
  } else {
    printf(1, "  FAIL: Could not acquire lock\n");
  }
  
  // Do some work while holding lock
  printf(1, "  Working with lock held...\n");
  for(int i = 0; i < 1000000; i++);
  
  if(release(lock_id) == 0){
    printf(1, "  PASS: Released lock %d\n", lock_id);
  } else {
    printf(1, "  FAIL: Could not release lock\n");
  }
  
  // Test 3: Two processes competing for same lock
  printf(1, "\nTest 3: Two processes competing for lock %d\n", lock_id);
  
  pid1 = fork();
  if(pid1 == 0){
    // Child 1
    printf(1, "  [Child 1 PID=%d] Attempting to acquire lock %d\n", getpid(), lock_id);
    lock(lock_id);
    printf(1, "  [Child 1 PID=%d] Acquired lock %d\n", getpid(), lock_id);
    
    // Hold lock for a while
    printf(1, "  [Child 1 PID=%d] Working with lock...\n", getpid());
    for(int i = 0; i < 5000000; i++);
    
    printf(1, "  [Child 1 PID=%d] Releasing lock %d\n", getpid(), lock_id);
    release(lock_id);
    exit();
  }
  
  // Small delay so child 1 acquires lock first
  sleep(5);
  
  pid2 = fork();
  if(pid2 == 0){
    // Child 2
    printf(1, "  [Child 2 PID=%d] Attempting to acquire lock %d (should wait)\n", getpid(), lock_id);
    lock(lock_id);
    printf(1, "  [Child 2 PID=%d] Acquired lock %d (after Child 1 released)\n", getpid(), lock_id);
    
    printf(1, "  [Child 2 PID=%d] Releasing lock %d\n", getpid(), lock_id);
    release(lock_id);
    exit();
  }
  
  wait();
  wait();
  
  printf(1, "\n=== TEST 4 Complete ===\n");
  printf(1, "Expected behavior:\n");
  printf(1, "  - Invalid lock IDs should be rejected\n");
  printf(1, "  - Lock acquire/release should work\n");
  printf(1, "  - Child 2 should wait for Child 1 to release lock\n\n");
  
  exit();
}