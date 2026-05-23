/* ToyFS public interface: a read-only in-memory teaching file system. */
#ifndef TOYOS_FS_H
#define TOYOS_FS_H

#include "types.h"
#include "param.h"

struct toyfs_file {
  const char *path;
  const char *data;
  uint64 size;
};

void fs_init(void);
int fs_read(const char *path, char *dst, int max);

#endif
