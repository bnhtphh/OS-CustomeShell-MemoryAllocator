#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // 1. Start up
  // 2. Create three mapped memory pages
  char *addr1 = mmap();
  char *addr2 = mmap();
  char *addr3 = mmap();
  
  // 3. Store test strings in each of the pages and print them
  strcpy(addr1, "Hello World!");
  strcpy(addr2, "hello from the child");
  strcpy(addr3, "Third child");
  printf("Parent: %s, %s, %s\n", addr1, addr2, addr3);
  
  // 4. Fork a child process
  int pid = fork();
  if(pid == 0) {
    
    // 5. Attempt to read the strings in the child, printing their values
    printf("Child reads: %s, %s, %s\n", addr1, addr2, addr3);
    
    // 6. Change the strings! Overwrite the first, 
    //    append to the second, and leave the third alone
    strcpy(addr1, "Overwrite 1");        
    strcpy(addr2, "Second page");    
    
    // 9. Fork - update the child process so that it forks and waits for another child (Mapception!)
    int pid2 = fork();
    if(pid2 == 0) {
      strcpy(addr3, "Modified by third string");
      exit(0);
    }
    
    wait(0); 
    exit(0);
  }
  
  // 7. Have the parent wait(0) for its child
  wait(0);
  
  // 8. Print the strings afterward - parent should see modified strings!
  printf("Parent after: %s, %s, %s\n", addr1, addr2, addr3);
  
  exit(0);
}
