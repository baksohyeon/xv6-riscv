#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
test_single_read(void)
{
  int fd;
  char buf[10];
  int initial_count, count_after_read;
  
  printf("Test 1: Single read operation\n");
  
  initial_count = getreadcount();
  printf("  Initial read count: %d\n", initial_count);
  
  fd = open("README", 0);
  if(fd < 0){
    printf("  ERROR: Failed to open README\n");
    exit(1);
  }
  
  read(fd, buf, 5);
  close(fd);
  
  count_after_read = getreadcount();
  printf("  Read count after read: %d\n", count_after_read);
  
  if(count_after_read == initial_count + 1){
    printf("  PASS: Read count incremented correctly\n");
  } else {
    printf("  FAIL: Expected %d, got %d\n", initial_count + 1, count_after_read);
  }
  printf("\n");
}

void
test_multiple_reads(void)
{
  int fd;
  char buf[10];
  int initial_count, count_after_reads;
  
  printf("Test 2: Multiple read operations\n");
  
  initial_count = getreadcount();
  printf("  Initial read count: %d\n", initial_count);
  
  fd = open("README", 0);
  if(fd < 0){
    printf("  ERROR: Failed to open README\n");
    exit(1);
  }
  
  read(fd, buf, 5);
  read(fd, buf, 5);
  read(fd, buf, 5);
  close(fd);
  
  count_after_reads = getreadcount();
  printf("  Read count after 3 reads: %d\n", count_after_reads);
  
  if(count_after_reads == initial_count + 3){
    printf("  PASS: Read count incremented correctly for multiple reads\n");
  } else {
    printf("  FAIL: Expected %d, got %d\n", initial_count + 3, count_after_reads);
  }
  printf("\n");
}

void
test_failed_read(void)
{
  int fd;
  int initial_count, count_after_failed;
  
  printf("Test 3: Failed read operation\n");
  
  initial_count = getreadcount();
  printf("  Initial read count: %d\n", initial_count);
  
  fd = open("nonexistent_file", 0);
  if(fd >= 0){
    printf("  WARNING: Expected file to not exist\n");
    close(fd);
  }
  
  count_after_failed = getreadcount();
  printf("  Read count after failed open: %d\n", count_after_failed);
  
  if(count_after_failed == initial_count){
    printf("  PASS: Read count unchanged after failed open\n");
  } else {
    printf("  FAIL: Expected %d, got %d\n", initial_count, count_after_failed);
  }
  printf("\n");
}

int
main(int argc, char *argv[])
{
  printf("Testing getreadcount() system call...\n\n");
  
  test_single_read();
  test_multiple_reads();
  test_failed_read();
  
  printf("All tests completed.\n");
  exit(0);
}