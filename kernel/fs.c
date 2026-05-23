/* ToyFS: read-only files backed by constant memory instead of a block device. */
#include "types.h"
#include "fs.h"
#include "defs.h"

static const char hello_data[] = "Hello from ToyFS!";

static const struct toyfs_file files[] = {
  {"/hello.txt", hello_data, sizeof(hello_data) - 1},
};

void fs_init(void) {
  printf("[ToyOS] ToyFS init ok, files=%d\n",
         (int)(sizeof(files) / sizeof(files[0])));
}

int fs_read(const char *path, char *dst, int max) {
  if (max < 0) {
    return -1;
  }
  for (uint i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (strcmp(path, files[i].path) == 0) {
      int n = (int)files[i].size;
      if (n > max) {
        n = max;
      }
      memmove(dst, files[i].data, (uint64)n);
      return n;
    }
  }
  return -1;
}
