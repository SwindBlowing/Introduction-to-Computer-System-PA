#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

//size_t ramdisk_read(void *buf, size_t offset, size_t len);
//size_t ramdisk_write(const void *buf, size_t offset, size_t len);
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  Elf_Ehdr ehdr;
  //ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));

  //check part
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  #if defined(__ISA_NATIVE__)
	assert(ehdr.e_machine == EM_X86_64);
  #elif defined(__ISA_RISCV__)
	assert(ehdr.e_machine == EM_RISCV);
  #endif
  //end check part

  Elf_Phdr phdr[ehdr.e_phnum];
  //ramdisk_read(phdr, ehdr.e_phoff, ehdr.e_phnum * sizeof(Elf_Phdr));
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read(fd, phdr, ehdr.e_phnum * sizeof(Elf_Phdr));
  for (size_t i = 0; i < ehdr.e_phnum; i++) {
	if (phdr[i].p_type == PT_LOAD) {
		size_t offset = phdr[i].p_offset;
		size_t virtAddr = phdr[i].p_vaddr;
		size_t fileSize = phdr[i].p_filesz;
		size_t memSize = phdr[i].p_memsz;
		//printf("%x %x %x %x\n", offset, virtAddr, fileSize, memSize);
		//ramdisk_read((void *)virtAddr, offset, fileSize);
		fs_lseek(fd, offset, SEEK_SET);
		fs_read(fd, (void *)virtAddr, fileSize);
		assert(memSize >= fileSize);
		memset((void *)(virtAddr + fileSize), 0, memSize - fileSize);
	}
  }
  fs_close(fd);
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

