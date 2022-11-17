#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char* current_filename, char *target_filename) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
    case T_FILE:
      if(0 == strcmp(current_filename, target_filename)) {
        fprintf(1, "%s\n", path);
      }
      break;

    case T_DIR:
      if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        printf("find: path too long\n");
        break;
      }
      strcpy(buf, path);
      p = buf+strlen(buf);
      *p++ = '/';
      while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
          continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
          find(buf, de.name, target_filename);
        }
      }
      break;
  }
  close(fd);
}

int
main(int argc, char *argv[]) {
  if(argc < 3) {
    fprintf(2, "find: too few arguments, expect: 2, got: %d\n", argc - 1);
    exit(-1);
  }

  if(argc > 3) {
    fprintf(2, "find: too many arguments, expect: 2, got: %d\n", argc - 1);
    exit(-1);
  }

  find(argv[1], argv[1], argv[2]);
  exit(-1);
}
