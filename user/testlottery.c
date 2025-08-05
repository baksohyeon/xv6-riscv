#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pstats.h"

// Test the stride scheduler with 3:2:1 ticket ratio
// Process A: 30 tickets, Process B: 20 tickets, Process C: 10 tickets

#define TEST_DURATION 100000  // Number of iterations to run
#define PROCESS_A_TICKETS 30
#define PROCESS_B_TICKETS 20  
#define PROCESS_C_TICKETS 10
#define SAMPLE_INTERVALS 10

// Global variables to store child PIDs for monitoring
int test_pids[3] = {0, 0, 0};

// Data collection for graphing
struct time_sample {
  int time_point;
  int ticks_a;
  int ticks_b; 
  int ticks_c;
  int total_ticks;
} samples[SAMPLE_INTERVALS];

int sample_count = 0;

void
worker_process(int process_id, int tickets)
{
  // Set the number of tickets for this process
  int result = settickets(tickets);
  if(result < 0) {
    fprintf(2, "Process %d: settickets FAILED!\n", process_id);
    exit(1);
  }
  
  volatile int counter = 0;
  while(1) {
    for(int j = 0; j < 10000; j++) {
      counter += j * j;
      counter %= 1000000;
    }
  }
  
  exit(0);
}

void
collect_sample_data(int time_point)
{
  struct pstat ps;
  int i;
  
  if(sample_count >= SAMPLE_INTERVALS) {
    return;
  }
  
  if(getpinfo(&ps) < 0) {
    fprintf(2, "Failed to get process info for sample\n");
    return;
  }
  
  samples[sample_count].time_point = time_point;
  samples[sample_count].ticks_a = 0;
  samples[sample_count].ticks_b = 0;
  samples[sample_count].ticks_c = 0;
  samples[sample_count].total_ticks = 0;
  
  int tickets_a = 0, tickets_b = 0, tickets_c = 0;
  int found_a = 0, found_b = 0, found_c = 0;
  
  for(i = 0; i < NPROC; i++) {
    if(ps.inuse[i]) {
      if(ps.pid[i] == test_pids[0]) {
        samples[sample_count].ticks_a = ps.ticks[i];
        tickets_a = ps.tickets[i];
        found_a = 1;
      } else if(ps.pid[i] == test_pids[1]) {
        samples[sample_count].ticks_b = ps.ticks[i];
        tickets_b = ps.tickets[i];
        found_b = 1;
      } else if(ps.pid[i] == test_pids[2]) {
        samples[sample_count].ticks_c = ps.ticks[i];
        tickets_c = ps.tickets[i];
        found_c = 1;
      }
    }
  }
  
  samples[sample_count].total_ticks = samples[sample_count].ticks_a + 
                                      samples[sample_count].ticks_b + 
                                      samples[sample_count].ticks_c;
  
  int pass_a = 0, pass_b = 0, pass_c = 0;
  for(i = 0; i < NPROC; i++) {
    if(ps.inuse[i]) {
      if(ps.pid[i] == test_pids[0]) {
        pass_a = ps.pass_value[i];
      } else if(ps.pid[i] == test_pids[1]) {
        pass_b = ps.pass_value[i];
      } else if(ps.pid[i] == test_pids[2]) {
        pass_c = ps.pass_value[i];
      }
    }
  }
  
  fprintf(1, "Sample %d (t=%d): A=%d(t%d,p%d)%s, B=%d(t%d,p%d)%s, C=%d(t%d,p%d)%s, Total=%d\n", 
          sample_count + 1, time_point,
          samples[sample_count].ticks_a, tickets_a, pass_a, found_a ? "" : "X",
          samples[sample_count].ticks_b, tickets_b, pass_b, found_b ? "" : "X", 
          samples[sample_count].ticks_c, tickets_c, pass_c, found_c ? "" : "X",
          samples[sample_count].total_ticks);
  
  sample_count++;
}


void
print_final_stats()
{
  struct pstat ps;
  int i;
  
  if(getpinfo(&ps) < 0) {
    fprintf(2, "Failed to get process info\n");
    return;
  }
  
  fprintf(1, "\n=== FINAL STRIDE SCHEDULER RESULTS ===\n");
  fprintf(1, "PID\tTickets\tTicks\tPercent\tExpected\n");
  fprintf(1, "---\t-------\t-----\t-------\t--------\n");
  
  int total_test_ticks = 0;
  int ticks_a = 0, ticks_b = 0, ticks_c = 0;
  
  for(i = 0; i < NPROC; i++) {
    if(ps.inuse[i] && ps.ticks[i] > 0) {
      if(ps.pid[i] == test_pids[0]) {
        ticks_a = ps.ticks[i];
        total_test_ticks += ticks_a;
      } else if(ps.pid[i] == test_pids[1]) {
        ticks_b = ps.ticks[i];
        total_test_ticks += ticks_b;
      } else if(ps.pid[i] == test_pids[2]) {
        ticks_c = ps.ticks[i];
        total_test_ticks += ticks_c;
      }
    }
  }
  
  if(total_test_ticks > 0) {
    int percent_a = (ticks_a * 100) / total_test_ticks;
    int percent_b = (ticks_b * 100) / total_test_ticks;
    int percent_c = (ticks_c * 100) / total_test_ticks;
    
    fprintf(1, "%d\t%d\t%d\t%d%%\t50%% [Process A]\n", test_pids[0], PROCESS_A_TICKETS, ticks_a, percent_a);
    fprintf(1, "%d\t%d\t%d\t%d%%\t33%% [Process B]\n", test_pids[1], PROCESS_B_TICKETS, ticks_b, percent_b);
    fprintf(1, "%d\t%d\t%d\t%d%%\t17%% [Process C]\n", test_pids[2], PROCESS_C_TICKETS, ticks_c, percent_c);
    
    fprintf(1, "\nTotal test process ticks: %d\n", total_test_ticks);
    fprintf(1, "Ticket ratio: %d:%d:%d (3:2:1)\n", PROCESS_A_TICKETS, PROCESS_B_TICKETS, PROCESS_C_TICKETS);
    fprintf(1, "Actual ratio: %d:%d:%d\n", ticks_a, ticks_b, ticks_c);
  }
}

int
main(int argc, char *argv[])
{
  int pid_a, pid_b, pid_c;
  
  fprintf(1, "=== STRIDE SCHEDULER TEST ===\n");
  fprintf(1, "Testing 3:2:1 ticket ratio (30:20:10 tickets)\n");
  fprintf(1, "Expected CPU distribution: 50%%:33%%:17%%\n\n");
  
  // Fork process A
  pid_a = fork();
  if(pid_a == 0) {
    worker_process(1, PROCESS_A_TICKETS);
  } else if(pid_a < 0) {
    fprintf(2, "Failed to fork process A\n");
    exit(1);
  }
  
  // Fork process B
  pid_b = fork();
  if(pid_b == 0) {
    worker_process(2, PROCESS_B_TICKETS);
  } else if(pid_b < 0) {
    fprintf(2, "Failed to fork process B\n");
    exit(1);
  }
  
  // Fork process C
  pid_c = fork();
  if(pid_c == 0) {
    worker_process(3, PROCESS_C_TICKETS);
  } else if(pid_c < 0) {
    fprintf(2, "Failed to fork process C\n");
    exit(1);
  }
  
  // Parent monitoring
  fprintf(1, "Forked processes: A=PID%d, B=PID%d, C=PID%d\n", pid_a, pid_b, pid_c);
  fprintf(1, "Process A: %d tickets, Process B: %d tickets, Process C: %d tickets\n", 
          PROCESS_A_TICKETS, PROCESS_B_TICKETS, PROCESS_C_TICKETS);
  
  // Store PIDs for monitoring
  test_pids[0] = pid_a;
  test_pids[1] = pid_b;
  test_pids[2] = pid_c;
  
  sleep(20);
  fprintf(1, "Starting sampling...\n");
  
  fprintf(1, "Progress: ");
  for(int i = 0; i < SAMPLE_INTERVALS; i++) {
    int sample_interval = 50 + (i * 10);  // 50, 60, 70, 80, 90, 100, 110, 120, 130, 140
    collect_sample_data(i * sample_interval);
    
    if(i % 2 == 0) {
      fprintf(1, "[%d%%] ", (i * 100) / SAMPLE_INTERVALS);
    }
    
    sleep(sample_interval);
  }
  fprintf(1, "[100%%]\n");
  
  fprintf(1, "\nSampling complete. Collecting final statistics...\n");
  print_final_stats();
  
  fprintf(1, "\nTerminating test processes...\n");
  kill(pid_a);
  kill(pid_b);
  kill(pid_c);
  
  int status;
  for(int i = 0; i < 3; i++) {
    int finished_pid = wait(&status);
    fprintf(1, "Process PID%d terminated\n", finished_pid);
  }
  exit(0);
}
