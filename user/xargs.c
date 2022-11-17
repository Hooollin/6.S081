#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

char buf[512];
char *args[MAXARG];

int
main(int argc, char *argv[]) {
  int i;
  int input_size;
  int status;
  int arg_idx;
  int new_arg_idx;
  int pid;
  if(argc < 2) {
    fprintf(2, "xargs: too few arguments.\n");
  }
  input_size = read(0, buf, 512);
  if(input_size < 0) {
    fprintf(2, "xargs: read error.\n");
    exit(-1);
  }

  if(input_size == 0) {
    exit(0);
  }

  arg_idx = 0;
  for(i = 1; i < argc; i++) {
    args[arg_idx++] = argv[i];
  }

  new_arg_idx = arg_idx;
  //fprintf(1, "%s\n\n", buf);

  // status: 0 -> space
  // status: 1 -> args
  // status: 2 -> newline
  status = 0;
  i = 0;
  while(i < input_size) {
    //fprintf(1, "ch: %c, pos: %d, status: %d, arg_idx: %d\n", buf[i], i, status, arg_idx);
    switch (status) {
      case 0:
        if(buf[i] == ' ') {
          ++i;
        }else if(buf[i] == '\n') {
          status = 2;
        }else{
          status = 1;
          args[arg_idx++] = &buf[i];
          ++i;
        }
        break;
      case 1:
        if(buf[i] == ' ') {
          buf[i] = '\0';
          status = 0;
          ++i;
        }else if(buf[i] == '\n') {
          buf[i] = '\0';
          status = 2;
        }else {
          ++i;
        }
        break;
      case 2:
        args[arg_idx] = 0;
        pid = fork();
        if(pid == 0) {
          // child
          //fprintf(1, "do exec: %s %s\n", args[0], args[1]);
          exec(args[0], args);
          fprintf(2, "xargs: exec failed\n");
          exit(-1);
        }else {
          wait(0);
          arg_idx = new_arg_idx;
          status = 0;
          i++;
        }
        break;
    }
  }
  exit(0);
}
