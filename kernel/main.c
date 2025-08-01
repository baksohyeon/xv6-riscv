#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if(cpuid() == 0){
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    printf("    ██████╗  ██████╗ ██████╗ ██╗████████╗ ██████╗ \n");
    printf("    ██╔══██╗██╔═══██╗██╔══██╗██║╚══██╔══╝██╔═══██╗\n");
    printf("    ██║  ██║██║   ██║██████╔╝██║   ██║   ██║   ██║\n");
    printf("    ██║  ██║██║   ██║██╔══██╗██║   ██║   ██║   ██║\n");
    printf("    ██████╔╝╚██████╔╝██║  ██║██║   ██║   ╚██████╔╝\n");
    printf("    ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝   ╚═╝    ╚═════╝ \n");
    printf("\n");
    printf("    ┌─────────────────────────────────────────────────┐\n");
    printf("    │              OS STUDY PROJECT                   │\n");
    printf("    │                                                 │\n");
    printf("    │  [KERNEL]     [SCHEDULER]     [MEMORY MGR]      │\n");
    printf("    │     │             │               │             │\n");
    printf("    │     ├─THREADS     ├─FCFS          ├─PAGING      │\n");
    printf("    │     ├─PROCESS     ├─SJF           ├─SEGMENTATION│\n");
    printf("    │     └─SYNC        └─PRIORITY      └─VIRTUAL MEM │\n");
    printf("    │                                                 │\n");
    printf("    │  STATUS: [ LEARNING IN PROGRESS... ]            │\n");
    printf("    └─────────────────────────────────────────────────┘\n");
    printf("\n");
    printf("\n");
    printf("\n");
    kinit();         // physical page allocator
    kvminit();       // create kernel page table
    kvminithart();   // turn on paging
    procinit();      // process table
    randominit();    // initialize random number generator for scheduling
    trapinit();      // trap vectors
    trapinithart();  // install kernel trap vector
    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
    binit();         // buffer cache
    iinit();         // inode table
    fileinit();      // file table
    virtio_disk_init(); // emulated hard disk
    userinit();      // first user process
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
  }

  scheduler();        
}
