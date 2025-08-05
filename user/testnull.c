#include "kernel/types.h"
#include "user/user.h"

int main() {
    printf("Testing null pointer dereference...\n");
    
    // This should cause a trap and kill the process
    int *null_ptr = 0;
    int value = *null_ptr;  // Dereference null pointer
    
    // This should never execute
    printf("ERROR: null pointer dereference did not trap! Value: %d\n", value);
    exit(1);
}
