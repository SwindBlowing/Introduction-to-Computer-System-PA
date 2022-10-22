#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

static size_t open_offset[10005] = {-1};

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode)
{
	printf("size: %u\n", file_table->size);
	for (int i = 0; i < file_table->size; i++)
		if (strcmp(pathname, file_table[i].name) == 0) {
			if (open_offset[i] == -1) open_offset[i] = file_table[i].disk_offset;
			return i;
		}
	panic("No file found");
	return -1;
}
size_t fs_read(int fd, void *buf, size_t len)
{
	len = len < file_table[fd].disk_offset + file_table[fd].size - open_offset[fd] ?
			len : file_table[fd].disk_offset + file_table[fd].size - open_offset[fd];
	ramdisk_read(buf, open_offset[fd], len);
	fs_lseek(fd, len, SEEK_CUR);
	return len;
}
size_t fs_write(int fd, const void *buf, size_t len)
{
	len = len < file_table[fd].disk_offset + file_table[fd].size - open_offset[fd] ?
			len : file_table[fd].disk_offset + file_table[fd].size - open_offset[fd];
	ramdisk_write(buf, open_offset[fd], len);
	fs_lseek(fd, len, SEEK_CUR);
	return len;
}
size_t fs_lseek(int fd, size_t offset, int whence)
{
	return -1;
}
int fs_close(int fd)
{
	return -1;
}