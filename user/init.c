/* First user process. It replaces itself with the demo shell. */
#include "user.h"

int main(void) {
  if (exec("sh") < 0) {
    printf("init: exec sh failed\n");
    exit(1);
  }
  return 0;
}
