#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

static size_t open_offset[10005] = {0};
static bool isOpen[10005] = {0};

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENT, FD_FB, FD_DISPINFO};

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
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_EVENT] = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_FB] = {"/dev/fb", 0, 0, dispinfo_read, fb_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int sys_w = io_read(AM_GPU_CONFIG).width;
  int sys_h = io_read(AM_GPU_CONFIG).height;
  file_table[FD_FB].size = sys_w * sys_h * 4;
}

int fs_open(const char *pathname, int flags, int mode)
{
	printf("Try to open %s\n", pathname);
	size_t nowSize = sizeof(file_table) / sizeof(Finfo);
	for (int i = 0; i < nowSize; i++)
		if (strcmp(pathname, file_table[i].name) == 0) {
			if (!isOpen[i]) {
				isOpen[i] = 1;
				fs_lseek(i, 0, SEEK_SET);
			}
			return i;
		}
	//panic("No file is found");
	return -2;
}
size_t fs_read(int fd, void *buf, size_t len)
{
	if (file_table[fd].read != NULL) return (file_table[fd].read)(buf, open_offset[fd], len);
	len = len < file_table[fd].size - open_offset[fd] ?
			len : file_table[fd].size - open_offset[fd];
	ramdisk_read(buf, file_table[fd].disk_offset + open_offset[fd], len);
	fs_lseek(fd, len, SEEK_CUR);
	return len;
}
size_t fs_write(int fd, const void *buf, size_t len)
{
	if (file_table[fd].write != NULL) return (file_table[fd].write)(buf, open_offset[fd], len);
	len = len < file_table[fd].size - open_offset[fd] ?
			len : file_table[fd].size - open_offset[fd];
	ramdisk_write(buf, file_table[fd].disk_offset + open_offset[fd], len);
	fs_lseek(fd, len, SEEK_CUR);
	return len;
}
size_t fs_lseek(int fd, size_t offset, int whence)
{
	switch (whence) {
		case SEEK_SET: open_offset[fd] = offset; break;
		case SEEK_CUR: open_offset[fd] = open_offset[fd] + offset; break;
		case SEEK_END: open_offset[fd] = file_table[fd].size + offset; break;
		default: panic("Invalid lseek whence!"); break;
	}
	if (open_offset[fd] >= file_table[fd].size)
		open_offset[fd] = file_table[fd].size;
	if (open_offset[fd] < 0) 
		open_offset[fd] = 0;
	return open_offset[fd];
}
int fs_close(int fd)
{
	isOpen[fd] = 0; fs_lseek(fd, 0, SEEK_SET);
	return 0;
}