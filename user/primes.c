#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void open_pipe(int *p) {
  if(pipe(p) < 0) {
    fprintf(2, "primes: pipe failed\n");
    exit(-1);
  }
}

void csp(int *p, int base) {
  int pid;
  int val;
  int cp[2];
  int forked = 0;
  close(p[1]);
  while(read(p[0], &val, 4) > 0) {
    if(val == base) {
      fprintf(1, "prime %d\n", val);
    }else if(val % base != 0) {
      if(!forked) {
        open_pipe(cp);
        pid = fork();
        if(pid == 0) {
          close(cp[1]);
          csp(cp, val);
        }else {
          close(cp[0]);
          forked = 1;
        }
      }
      write(cp[1], &val, sizeof(val));
    }
  }
  close(p[0]);
  if(forked) {
    close(cp[1]);
  }
  wait(&pid);
}

int
main(int argc, char *argv[]) {
  int i;
  int pid;
  int p[2];
  if(argc > 1) {
    fprintf(2, "primes: too many arguments, expect: 0, got: %d\n", argc);
    exit(-1);
  }

  open_pipe(p);

  pid = fork();
  if(pid == 0){
    csp(p, 2);
  }else {
    close(p[0]);
    for(i = 2; i <= 35; i++) {
      write(p[1], &i, sizeof(i));
    }
    close(p[1]);
    wait(&pid);
  }
  exit(0);
}
