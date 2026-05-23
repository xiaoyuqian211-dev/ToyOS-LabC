/* User program that reads /hello.txt from ToyFS through SYS_readfile. */
#include "user.h"

int main(void) {
  char buf[128];
  printf("user: file system test\n");
  int n = readfile("/hello.txt", buf, sizeof(buf) - 1);
  if (n < 0) {
    printf("cat /hello.txt: read failed\n");
    return 1;
  }
  buf[n] = 0;
  printf("cat /hello.txt: %s\n", buf);
  return 0;
}
