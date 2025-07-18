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

void
worker_process(int worker_id, int reads_per_worker)
{
  int fd;
  char buf[10];
  int i;
  
  fd = open("README", 0);
  if(fd < 0){
    printf("  Worker %d: Failed to open README\n", worker_id);
    exit(1);
  }
  
  for(i = 0; i < reads_per_worker; i++){
    read(fd, buf, 1);
  }
  
  close(fd);
  exit(0);
}

void
test_multicore_stress(void)
{
  int initial_count, final_count;
  int num_workers = 10;
  int reads_per_worker = 1000;
  int total_expected_reads = num_workers * reads_per_worker;
  int i, pid;
  int status;
  
  printf("Test 4: Multicore stress test (%d processes x %d reads = %d total)\n", 
         num_workers, reads_per_worker, total_expected_reads);
  
  initial_count = getreadcount();
  printf("  Initial read count: %d\n", initial_count);
  
  for(i = 0; i < num_workers; i++){
    pid = fork();
    if(pid == 0){
      worker_process(i, reads_per_worker);
    } else if(pid < 0){
      printf("  ERROR: Failed to fork worker %d\n", i);
      exit(1);
    }
  }
  
  for(i = 0; i < num_workers; i++){
    wait(&status);
  }
  
  final_count = getreadcount();
  printf("  Final read count: %d\n", final_count);
  printf("  Expected increment: %d\n", total_expected_reads);
  printf("  Actual increment: %d\n", final_count - initial_count);
  
  if(final_count == initial_count + total_expected_reads){
    printf("  PASS: All %d reads counted correctly in multicore test\n", total_expected_reads);
  } else {
    printf("  FAIL: Expected %d, got %d (difference: %d)\n", 
           initial_count + total_expected_reads, final_count, 
           (final_count - initial_count) - total_expected_reads);
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
  test_multicore_stress();
  
  printf("All tests completed.\n");
  exit(0);
}