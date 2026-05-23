/* User program that demonstrates fork, wait and cooperative scheduling. */
#include "user.h"

int main(void) {
  printf("user: fork test start\n");
  int pid = fork();
  if (pid < 0) {
    printf("forkdemo: fork failed\n");
    return 1;
  }
  if (pid == 0) {
    printf("child: hello\n");
    yield();
    exit(0);
  }

  printf("parent: child pid = %d\n", pid);
  int status = 0;
  int done = wait(&status);
  printf("parent: wait pid = %d status = %d\n", done, status);
  return 0;
}
