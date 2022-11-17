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
  int p[2];
  int pid;
  char buf[1];
  if(argc > 1) {
    fprintf(2, "pingpong: too many arguments, expect: 0, got: %d\n", argc);
    exit(-1);
  }
  
  open_pipe(p);

  pid = fork();
  if(pid == 0) {
    // child
    read(p[0], buf, 1);
    close(p[0]);
    
    pid = getpid();
    fprintf(1, "%d: received ping\n", pid);

    write(p[1], "", 1);
    close(p[1]);
  }else {
    // parent
    write(p[1], "", 1);
    close(p[1]);
    
    // must wait, otherwize read previously readed data
    wait(0);

    read(p[0], buf, 1);
    close(p[0]);

    pid = getpid();
    fprintf(1, "%d: received pong\n", pid);
  }
  exit(0);
}
