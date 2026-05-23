/* User program that demonstrates write/getpid/yield syscalls. */
#include "user.h"

int main(void) {
  printf("user: hello from user program\n");
  printf("user: syscall write works\n");
  printf("user: my pid is %d\n", getpid());
  yield();
  return 0;
}
