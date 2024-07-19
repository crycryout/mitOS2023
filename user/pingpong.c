#include "kernel/types.h"
#include "user/user.h"
int main(int argc, char *argv[]) {
  char receive;
  int pp1[2];
  int pp2[2];
  pipe(pp1);
  pipe(pp2);
  if (fork() == 0) {
    close(pp1[0]);
    close(pp2[1]);
    write(pp1[1], "a", 1);
    close(pp1[1]);
    read(pp2[0], &receive, 1);
    close(pp2[0]);
    fprintf(1, "%d: received ping\n", getpid());
  } else {
    close(pp2[0]);
    close(pp1[1]);
    write(pp2[1], "b", 1);
    close(pp2[1]);
    read(pp1[0], &receive, 1);
    close(pp1[0]);
    wait(0);
    fprintf(1, "%d: received pong\n", getpid());
  }
  exit(0);
}
