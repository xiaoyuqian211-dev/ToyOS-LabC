/* A tiny scripted shell: commands are built in, then executed as demos. */
#include "user.h"

static int spawn_and_wait(const char *prog) {
  int pid = fork();
  if (pid < 0) {
    printf("sh: fork failed for %s\n", prog);
    return -1;
  }
  if (pid == 0) {
    if (exec(prog) < 0) {
      printf("sh: exec %s failed\n", prog);
      exit(1);
    }
  }
  int status = 0;
  int done = wait(&status);
  if (done < 0) {
    printf("sh: wait failed\n");
    return -1;
  }
  return status;
}

static void run_command(const char *cmd) {
  printf("sh$ %s\n", cmd);
  if (strcmp(cmd, "help") == 0) {
    puts("builtins: help hello forkdemo cat hello.txt ps info exit");
  } else if (strcmp(cmd, "hello") == 0) {
    spawn_and_wait("hello");
  } else if (strcmp(cmd, "forkdemo") == 0) {
    spawn_and_wait("forkdemo");
  } else if (strcmp(cmd, "cat hello.txt") == 0) {
    spawn_and_wait("cat");
  } else if (strcmp(cmd, "ps") == 0 || strcmp(cmd, "info") == 0) {
    info();
  } else if (strcmp(cmd, "exit") == 0) {
    exit(0);
  } else {
    printf("sh: unknown command: %s\n", cmd);
  }
}

int main(void) {
  const char *script[] = {
    "help",
    "hello",
    "forkdemo",
    "cat hello.txt",
    "ps",
    "exit",
  };

  for (uint i = 0; i < sizeof(script) / sizeof(script[0]); i++) {
    run_command(script[i]);
    yield();
  }
  exit(0);
}
