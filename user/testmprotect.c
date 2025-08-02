#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

// Global variable in data section
int global_var = 42;

void test_basic_mprotect() {
    printf("=== Testing basic mprotect functionality ===\n");
    
    // Test mprotect on a data page
    char *data = sbrk(4096);  // Allocate one page
    if (data == (char*)-1) {
        printf("FAIL: sbrk failed\n");
        return;
    }
    
    // Initialize the data
    *data = 'A';
    printf("Initial data: %c\n", *data);
    
    // Make the page read-only
    if (mprotect(data, 1) < 0) {
        printf("FAIL: mprotect failed\n");
        return;
    }
    printf("SUCCESS: mprotect succeeded\n");
    
    // Reading should still work
    printf("After mprotect, data: %c\n", *data);
    
    printf("Now attempting to write to protected page...\n");
    // This should cause a trap and kill the process
    *data = 'B';
    
    // This should never execute
    printf("ERROR: Write to protected page succeeded! This should not happen.\n");
}

void test_munprotect() {
    printf("=== Testing munprotect functionality ===\n");
    
    // Allocate a page
    char *data = sbrk(4096);
    if (data == (char*)-1) {
        printf("FAIL: sbrk failed\n");
        return;
    }
    
    *data = 'X';
    printf("Initial data: %c\n", *data);
    
    // Protect it
    if (mprotect(data, 1) < 0) {
        printf("FAIL: mprotect failed\n");
        return;
    }
    
    // Unprotect it
    if (munprotect(data, 1) < 0) {
        printf("FAIL: munprotect failed\n");
        return;
    }
    printf("SUCCESS: munprotect succeeded\n");
    
    // Writing should work now
    *data = 'Y';
    printf("After munprotect, data: %c\n", *data);
    printf("SUCCESS: Write to unprotected page succeeded\n");
}

void test_parameter_validation() {
    printf("=== Testing parameter validation ===\n");
    
    // Test invalid parameters
    if (mprotect((void*)1, 1) == -1) {
        printf("SUCCESS: mprotect correctly rejected unaligned address\n");
    } else {
        printf("FAIL: mprotect should reject unaligned address\n");
    }
    
    if (mprotect((void*)0x1000, 0) == -1) {
        printf("SUCCESS: mprotect correctly rejected len <= 0\n");
    } else {
        printf("FAIL: mprotect should reject len <= 0\n");
    }
    
    if (mprotect((void*)0x80000000, 1) == -1) {
        printf("SUCCESS: mprotect correctly rejected address outside address space\n");
    } else {
        printf("FAIL: mprotect should reject address outside address space\n");
    }
}

void test_fork_inheritance() {
    printf("=== Testing fork inheritance ===\n");
    
    // Allocate and protect a page in parent
    char *data = sbrk(4096);
    if (data == (char*)-1) {
        printf("FAIL: sbrk failed\n");
        return;
    }
    
    *data = 'P';  // P for parent
    
    if (mprotect(data, 1) < 0) {
        printf("FAIL: mprotect failed in parent\n");
        return;
    }
    
    int pid = fork();
    if (pid == 0) {
        // Child process
        printf("In child: attempting to write to inherited protected page...\n");
        *data = 'C';  // This should trap
        printf("ERROR: Child write succeeded! Protection not inherited.\n");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        wait(&status);
        printf("Child process completed (likely trapped as expected)\n");
    } else {
        printf("FAIL: fork failed\n");
    }
}

int main() {
    printf("Starting memory protection tests...\n\n");
    
    // Test parameter validation first (safe)
    test_parameter_validation();
    printf("\n");
    
    // Test munprotect (safe)
    test_munprotect();
    printf("\n");
    
    // Test fork inheritance (child will trap)
    test_fork_inheritance();
    printf("\n");
    
    // Test basic mprotect last (this will trap and kill the process)
    printf("WARNING: Next test will cause this process to trap and exit\n");
    test_basic_mprotect();
    
    // Should never reach here
    exit(0);
}
