#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pstats.h"

// Test the lottery scheduler with 3:2:1 ticket ratio
// Process A: 30 tickets, Process B: 20 tickets, Process C: 10 tickets

#define TEST_DURATION 1000  // Number of iterations to run
#define PROCESS_A_TICKETS 30
#define PROCESS_B_TICKETS 20  
#define PROCESS_C_TICKETS 10

void
worker_process(char process_name, int tickets)
{
  int i;
  
  // Set the number of tickets for this process
  if(settickets(tickets) < 0) {
    fprintf(2, "Failed to set tickets for process %c\n", process_name);
    exit(1);
  }
  
  fprintf(2, "Process %c: Set tickets to %d, PID=%d\n", process_name, tickets, getpid());
  
  // Do some work to consume CPU time
  for(i = 0; i < TEST_DURATION; i++) {
    // Busy work - just consume CPU cycles
    volatile int dummy = 0;
    for(int j = 0; j < 10000; j++) {
      dummy += j;
    }
    
    // Yield occasionally to allow scheduler to make decisions
    if(i % 100 == 0) {
      sleep(1);  // Brief sleep to allow other processes to run
    }
  }
  
  fprintf(1, "Process %c: Finished work\n", process_name);
  exit(0);
}

void
print_process_stats()
{
  struct pstat ps;
  int i;
  
  if(getpinfo(&ps) < 0) {
    fprintf(2, "Failed to get process info\n");
    return;
  }
  
  fprintf(1, "\nProcess Statistics:\n");
  fprintf(1, "PID\tTickets\tTicks\tRatio\n");
  fprintf(1, "---\t-------\t-----\t-----\n");
  
  int total_ticks = 0;
  for(i = 0; i < NPROC; i++) {
    if(ps.inuse[i] && ps.ticks[i] > 0) {
      total_ticks += ps.ticks[i];
    }
  }
  
  for(i = 0; i < NPROC; i++) {
    if(ps.inuse[i] && ps.ticks[i] > 0) {
      int ratio = total_ticks > 0 ? (ps.ticks[i] * 100) / total_ticks : 0;
      fprintf(1, "%d\t%d\t%d\t%d%%\n", ps.pid[i], ps.tickets[i], ps.ticks[i], ratio);
    }
  }
  fprintf(1, "\nTotal ticks: %d\n", total_ticks);
}

int
main(int argc, char *argv[])
{
  int pid_a, pid_b, pid_c;
  
  fprintf(1, "Lottery Scheduler Test - 3:2:1 Ticket Ratio\n");
  fprintf(1, "Process A: %d tickets\n", PROCESS_A_TICKETS);
  fprintf(1, "Process B: %d tickets\n", PROCESS_B_TICKETS);
  fprintf(1, "Process C: %d tickets\n", PROCESS_C_TICKETS);
  fprintf(1, "Expected ratio: 3:2:1 (50%%:33.3%%:16.7%%)\n\n");
  
  // Fork process A
  pid_a = fork();
  if(pid_a == 0) {
    worker_process('A', PROCESS_A_TICKETS);
  } else if(pid_a < 0) {
    fprintf(2, "Failed to fork process A\n");
    exit(1);
  }
  
  // Fork process B
  pid_b = fork();
  if(pid_b == 0) {
    worker_process('B', PROCESS_B_TICKETS);
  } else if(pid_b < 0) {
    fprintf(2, "Failed to fork process B\n");
    exit(1);
  }
  
  // Fork process C
  pid_c = fork();
  if(pid_c == 0) {
    worker_process('C', PROCESS_C_TICKETS);
  } else if(pid_c < 0) {
    fprintf(2, "Failed to fork process C\n");
    exit(1);
  }
  
  // Parent process waits for all children to complete
  fprintf(1, "Waiting for processes to complete...\n");
  
  wait(0);  // Wait for process A
  wait(0);  // Wait for process B  
  wait(0);  // Wait for process C
  
  fprintf(1, "\nAll processes completed. Final statistics:\n");
  print_process_stats();
  
  fprintf(1, "\nLottery scheduler test completed.\n");
  fprintf(1, "Expected: Process A should get ~50%%, B ~33%%, C ~17%% of CPU time\n");
  
  exit(0);
}
