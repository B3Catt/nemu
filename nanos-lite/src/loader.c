#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

void ramdisk_read(void *buf, off_t offset, size_t len);

size_t get_ramdisk_size();

extern size_t fs_filesz(int fd);
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, as->ptr, fs_filesz(fd));
  fs_close(fd);
  return (uintptr_t)as->ptr;
}
