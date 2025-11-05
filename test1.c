#include "types.h"
#include "stat.h"
#include "user.h"

// Assignment pseudocode:
// for i = 1 to infinity:
//   prime = true
//   for j = 2 to sqrt(i)
//     if i % j == 0
//       prime = false
//       break
//   if prime:
//     print(i)

void
findprimes(int childnum)
{
  int i, j;
  int prime;
  int count = 0;
  
  // for i = 1 to infinity
  for(i = 1; i < 10000; i++){
    prime = 1;  // prime = true
    
    // for j = 2 to sqrt(i)
    for(j = 2; j * j <= i; j++){
      if(i % j == 0){  // if i % j == 0
        prime = 0;     // prime = false
        break;         // break
      }
    }
    
    // if prime: print(i)
    if(prime && i > 1){
      count++;
    }
  }
  
  printf(1, "Child %d (Priority %d): found %d primes\n", 
         childnum, 
         childnum == 1 ? 1 : (childnum == 2 ? 3 : 5), 
         count);
  exit();
}

int
main(void)
{
  int pid1, pid2, pid3;
  
  printf(1, "\n=== TEST 1: Prime Number Finder ===\n");
  printf(1, "Three processes finding primes up to 10000\n\n");
  
  pid1 = fork();
  if(pid1 == 0){
    nice(getpid(), 1);
    findprimes(1);
  }
  
  pid2 = fork();
  if(pid2 == 0){
    nice(getpid(), 3);
    findprimes(2);
  }
  
  pid3 = fork();
  if(pid3 == 0){
    nice(getpid(), 5);
    findprimes(3);
  }
  
  wait();
  wait();
  wait();
  
  printf(1, "\n=== TEST 1 Complete ===\n");
  printf(1, "Expected: Priority 1 finishes first, Priority 5 finishes last\n\n");
  exit();
}