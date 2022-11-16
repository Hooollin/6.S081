#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) 
{
  int t;
  if(argc > 2) {
    fprintf(2, "sleep: too many arguments: expect: 1, got: %d\n", argc - 1);
    exit(-1);
  }

  // no except
  t = atoi(argv[1]);
  sleep(t);

  exit(0);
}
