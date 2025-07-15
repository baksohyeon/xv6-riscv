#include "kernel/types.h"
#include "user/user.h"

void
main(int argc, char *argv[])
{

  fprintf(1, "Hello, World!\n");
  if (argc > 1) {
    fprintf(2, "Hello, My name is '%s'\n", argv[1]);
    exit(1);
  }

  exit(0);
}
