# Stride Scheduler Implementation for xv6

This implementation adds a **stride scheduling algorithm** to xv6, replacing the default round-robin scheduler with a proportional-share scheduler that provides CPU time proportional to the number of tickets each process holds.

## Overview

The stride scheduler ensures that processes receive CPU time in proportion to their ticket allocation:
- Process with **30 tickets** → ~50% CPU time
- Process with **20 tickets** → ~33% CPU time  
- Process with **10 tickets** → ~17% CPU time

This creates a **3:2:1 ratio** as specified in the assignment requirements.

## Key Features

### 1. **Stride Scheduling Algorithm**
- Each process has a `pass_value` that increases by `stride = 10000/tickets` when scheduled
- Scheduler always picks the process with the minimum `pass_value`
- Provides proportional CPU allocation based on ticket count

### 2. **Overflow Prevention**
- Periodic normalization of pass values to prevent integer overflow
- Automatic adjustment when values become too large

### 3. **Starvation Prevention**
- New processes inherit appropriate pass values
- Dynamic adjustment when ticket allocation changes significantly
- Minimum pass value tracking across system

### 4. **Optimized Timer Preemption**
- Smart preemption logic that only yields when beneficial
- Reduced unnecessary context switches
- Better responsiveness while maintaining proportional scheduling

## Implementation Details

### Modified Files

#### `kernel/proc.h`
```c
struct proc {
    // ... existing fields ...
    uint tickets;        // Number of tickets for lottery scheduling
    uint pass_value;     // Pass value for stride scheduling  
    uint64 ticks;        // Number of times scheduled
};

#define DEFAULT_TICKETS 1
```

#### `kernel/proc.c`
- `scheduler()`: Main stride scheduling loop
- `get_min_pass_proc()`: Finds process with minimum pass value
- `update_pass()`: Updates pass value when process runs
- `normalize_pass_values()`: Prevents overflow
- `settickets()`: System call to set process tickets
- `getpinfo()`: System call to get process statistics

#### `kernel/trap.c`
- Smart timer preemption logic
- Selective yielding based on pass value differences

#### `kernel/exec.c`
- Proper initialization of stride parameters for new programs

## Testing and Visualization

### Test Program: `user/testlottery.c`

The test program creates three CPU-intensive processes with a 3:2:1 ticket ratio:

```c
#define PROCESS_A_TICKETS 30  // Expected: ~50% CPU
#define PROCESS_B_TICKETS 20  // Expected: ~33% CPU  
#define PROCESS_C_TICKETS 10  // Expected: ~17% CPU
```

### Graph Generation

A Python script generates visualizations of scheduler performance:

**`plot_scheduler_results.py`** creates:
1. **Cumulative ticks over time** - Shows total CPU usage
2. **CPU percentage distribution** - Shows proportional allocation
3. **Stacked area chart** - Visual distribution over time
4. **Final comparison** - Actual vs expected results

## Usage Instructions

### 1. Build and Run Test

```bash
# Build xv6 with stride scheduler
make clean && make

# Run xv6
make qemu

# In xv6 shell, run the test
$ testlottery
```

### 2. Generate Graphs

```bash
# Save test output to file
$ testlottery > results.txt

# Generate graphs (requires Python3 + matplotlib + numpy)
$ python3 plot_scheduler_results.py results.txt
```

### 3. Automated Testing (Alternative)

```bash
# Use the provided script
./run_stride_test.sh
```

## Expected Results

### CPU Distribution
- **Process A (30 tickets)**: ~50% CPU time
- **Process B (20 tickets)**: ~33% CPU time
- **Process C (10 tickets)**: ~17% CPU time

### Performance Metrics
- **Fairness**: Deviation from expected ratios should be < 5%
- **Responsiveness**: Processes should get scheduled regularly
- **Efficiency**: Minimal overhead from scheduling algorithm

## System Calls Added

### `settickets(int number)`
Sets the number of tickets for the calling process.
- **Returns**: 0 on success, -1 on error
- **Usage**: `settickets(30);`

### `getpinfo(struct pstat* ps)`
Retrieves scheduling statistics for all processes.
- **Returns**: 0 on success, -1 on error
- **Usage**: Used by test program to collect data

## Technical Notes

### Pass Value Calculation
```c
// When a process is scheduled:
pass_value += (10000 / tickets);
```

### Scheduler Selection Logic
```c
// Find process with minimum pass value
for(p = proc; p < &proc[NPROC]; p++) {
    if(p->state == RUNNABLE && p->pass_value < min_pass) {
        min_pass = p->pass_value;
        min_proc = p;
    }
}
```

### Timer Preemption Optimization
```c
// Only yield if significantly beneficial
if(min_pass + stride < current_pass) {
    yield();
}
```

## Files Overview

| File | Purpose |
|------|---------|
| `kernel/proc.c` | Core stride scheduler implementation |
| `kernel/proc.h` | Process structure with stride fields |
| `kernel/trap.c` | Timer interrupt handling |
| `kernel/exec.c` | Process initialization |
| `user/testlottery.c` | Test program for validation |
| `plot_scheduler_results.py` | Graph generation script |
| `run_stride_test.sh` | Automated test runner |

## Dependencies for Graphing

```bash
# Install Python dependencies
pip3 install matplotlib numpy

# Or using system package manager
sudo apt-get install python3-matplotlib python3-numpy  # Ubuntu/Debian
brew install python3-matplotlib python3-numpy          # macOS
```

## Troubleshooting

### Common Issues

1. **Build Errors**: Ensure all modified files are properly saved
2. **Test Doesn't Run**: Check that `testlottery` is included in Makefile
3. **No Graphs Generated**: Verify Python dependencies are installed
4. **Unexpected Results**: Let test run for full duration (2-3 minutes)

### Debugging

- Use `printf` statements in kernel code to trace scheduling decisions
- Check process statistics with `getpinfo()` system call
- Monitor pass values during execution

## Performance Characteristics

- **Time Complexity**: O(n) for each scheduling decision
- **Space Complexity**: O(1) additional space per process  
- **Fairness**: Proportional allocation with bounded error
- **Latency**: Low scheduling overhead with smart preemption

This implementation demonstrates a production-quality stride scheduler suitable for systems requiring proportional CPU allocation.
