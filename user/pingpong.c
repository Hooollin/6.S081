#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void open_pipe(int *p) {
  if(pipe(p) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(-1);
  }
}

int
main(int argc, char *argv[]) {
  int pp[2];
  int cp[2];
  int pid;
  char buf[1];
  if(argc > 1) {
    fprintf(2, "pingpong: too many arguments, expect: 0, got: %d\n", argc);
    exit(-1);
  }

  open_pipe(pp);
  open_pipe(cp);

  pid = fork();
  if(pid == 0) {
    // child
    close(cp[0]);

    read(pp[0], buf, 1);
    close(pp[0]);
    
    pid = getpid();
    fprintf(1, "%d: received ping\n", pid);

    write(cp[1], "", 1);
    close(cp[1]);

  }else {
    // parent
    close(pp[0]);

    write(pp[1], "", 1);
    close(pp[1]);

    read(cp[0], buf, 1);
    close(cp[0]);

    pid = getpid();
    fprintf(1, "%d: received pong\n", pid);
  }
  exit(0);
}
