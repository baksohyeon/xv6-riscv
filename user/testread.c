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
aggressive_worker(int worker_id, int reads_per_worker)
{
  int fd;
  char buf[1];
  int i;
  
  // Open file
  fd = open("README", 0);
  if(fd < 0){
    printf("  Worker %d: Failed to open README\n", worker_id);
    exit(1);
  }
  
  // Perform rapid reads to stress test race conditions
  for(i = 0; i < reads_per_worker; i++){
    read(fd, buf, 1);
    // No delay - maximize concurrency pressure
  }
  
  close(fd);
  exit(0);
}

void
test_race_condition_stress(void)
{
  int initial_count, final_count;
  int num_workers = 50;  // More processes for higher contention
  int reads_per_worker = 100;  // Fewer reads per worker but more workers
  int total_expected_reads = num_workers * reads_per_worker;
  int i, pid;
  int status;
  
  printf("Test 4: Race condition stress test (%d processes x %d reads = %d total)\n", 
         num_workers, reads_per_worker, total_expected_reads);
  
  initial_count = getreadcount();
  printf("  Initial read count: %d\n", initial_count);
  
  // Fork all workers quickly to maximize concurrent execution
  for(i = 0; i < num_workers; i++){
    pid = fork();
    if(pid == 0){
      aggressive_worker(i, reads_per_worker);
    } else if(pid < 0){
      printf("  ERROR: Failed to fork worker %d\n", i);
      exit(1);
    }
  }
  
  // Wait for all workers to complete
  for(i = 0; i < num_workers; i++){
    wait(&status);
  }
  
  final_count = getreadcount();
  printf("  Final read count: %d\n", final_count);
  printf("  Expected increment: %d\n", total_expected_reads);
  printf("  Actual increment: %d\n", final_count - initial_count);
  
  if(final_count == initial_count + total_expected_reads){
    printf("  PASS: All %d reads counted correctly in race condition test\n", total_expected_reads);
  } else {
    printf("  FAIL: Expected %d, got %d (lost %d reads due to race conditions)\n", 
           initial_count + total_expected_reads, final_count, 
           total_expected_reads - (final_count - initial_count));
  }
  printf("\n");
}

void
burst_worker(int worker_id, int burst_size)
{
  int fd;
  char buf[1];
  int i;
  int count_before, count_after;
  
  fd = open("README", 0);
  if(fd < 0){
    exit(1);
  }
  
  count_before = getreadcount();
  
  // Perform burst of reads
  for(i = 0; i < burst_size; i++){
    read(fd, buf, 1);
  }
  
  count_after = getreadcount();
  
  // Check if our reads were counted correctly
  if(count_after - count_before != burst_size){
    printf("  Worker %d: Expected %d, got %d (lost %d)\n", 
           worker_id, burst_size, count_after - count_before,
           burst_size - (count_after - count_before));
  }
  
  close(fd);
  exit(0);
}

void
test_concurrent_bursts(void)
{
  int initial_count, final_count;
  int num_workers = 20;
  int burst_size = 50;
  int total_expected_reads = num_workers * burst_size;
  int i, pid;
  int status;
  
  printf("Test 5: Concurrent burst test (%d processes x %d reads = %d total)\n", 
         num_workers, burst_size, total_expected_reads);
  
  initial_count = getreadcount();
  printf("  Initial read count: %d\n", initial_count);
  
  // Fork all workers
  for(i = 0; i < num_workers; i++){
    pid = fork();
    if(pid == 0){
      burst_worker(i, burst_size);
    } else if(pid < 0){
      printf("  ERROR: Failed to fork burst worker %d\n", i);
      exit(1);
    }
  }
  
  // Wait for all workers
  for(i = 0; i < num_workers; i++){
    wait(&status);
  }
  
  final_count = getreadcount();
  printf("  Final read count: %d\n", final_count);
  printf("  Expected increment: %d\n", total_expected_reads);
  printf("  Actual increment: %d\n", final_count - initial_count);
  
  if(final_count == initial_count + total_expected_reads){
    printf("  PASS: All %d reads counted correctly in concurrent burst test\n", total_expected_reads);
  } else {
    printf("  FAIL: Expected %d, got %d (lost %d reads)\n", 
           initial_count + total_expected_reads, final_count, 
           total_expected_reads - (final_count - initial_count));
  }
  printf("\n");
}

void
mixed_workload_worker(int worker_id)
{
  int fd;
  char buf[10];
  int i;
  
  fd = open("README", 0);
  if(fd < 0){
    exit(1);
  }
  
  // Mix of different read sizes and patterns
  for(i = 0; i < 20; i++){
    read(fd, buf, 1);      // Small read
    read(fd, buf, 5);      // Medium read
    read(fd, buf, 10);     // Larger read
    getreadcount();        // Query count during execution
  }
  
  close(fd);
  exit(0);
}

void
test_mixed_concurrent_workload(void)
{
  int initial_count, final_count;
  int num_workers = 15;
  int expected_reads_per_worker = 60; // 3 reads * 20 iterations
  int total_expected_reads = num_workers * expected_reads_per_worker;
  int i, pid;
  int status;
  
  printf("Test 6: Mixed concurrent workload (%d processes x %d reads = %d total)\n", 
         num_workers, expected_reads_per_worker, total_expected_reads);
  
  initial_count = getreadcount();
  printf("  Initial read count: %d\n", initial_count);
  
  // Fork mixed workload workers
  for(i = 0; i < num_workers; i++){
    pid = fork();
    if(pid == 0){
      mixed_workload_worker(i);
    } else if(pid < 0){
      printf("  ERROR: Failed to fork mixed worker %d\n", i);
      exit(1);
    }
  }
  
  // Wait for all workers
  for(i = 0; i < num_workers; i++){
    wait(&status);
  }
  
  final_count = getreadcount();
  printf("  Final read count: %d\n", final_count);
  printf("  Expected increment: %d\n", total_expected_reads);
  printf("  Actual increment: %d\n", final_count - initial_count);
  
  if(final_count == initial_count + total_expected_reads){
    printf("  PASS: All %d reads counted correctly in mixed workload test\n", total_expected_reads);
  } else {
    printf("  FAIL: Expected %d, got %d (lost %d reads)\n", 
           initial_count + total_expected_reads, final_count, 
           total_expected_reads - (final_count - initial_count));
  }
  printf("\n");
}

int
main(int argc, char *argv[])
{
  printf("Testing getreadcount() system call for concurrency issues...\n\n");
  
  test_single_read();
  test_multiple_reads();
  test_failed_read();
  test_race_condition_stress();
  test_concurrent_bursts();
  test_mixed_concurrent_workload();
  
  printf("All concurrency tests completed.\n");
  printf("If any tests show lost reads, there may be race conditions in the kernel implementation.\n");
  exit(0);
}
