#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

void ramdisk_read(void *buf, off_t offset, size_t len);

size_t get_ramdisk_size();

extern size_t fs_filesz(int fd);
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);
void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  //fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));

  void *pa, *va;
  int page_count = fs_filesz(fd) / 4096 + 1;

  for (int i = 0; i < page_count; i++) {
    va = DEFAULT_ENTRY + (i << 12);
    pa = new_page();
    Log("Map va to pa: 0x%08x to 0x%08x",va,pa);
    _map(as, va, pa);
    fs_read(fd, pa, 4096);
  }

  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
