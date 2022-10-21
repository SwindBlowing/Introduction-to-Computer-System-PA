#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  #ifdef __LP64__
  panic("?");
  #endif
  Elf_Ehdr ehdr;
  Elf_Phdr phdr;
  ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  for (size_t i = 0; i < ehdr.e_phnum; i++) {
	ramdisk_read(&phdr, ehdr.e_phoff + i * sizeof(Elf_Phdr) * 8, sizeof(Elf_Phdr));
	if (phdr.p_type == PT_LOAD) {
		size_t offset = phdr.p_offset;
		size_t virtAddr = phdr.p_vaddr;
		size_t fileSize = phdr.p_filesz;
		size_t memSize = phdr.p_memsz;
		ramdisk_read((void *)virtAddr, offset, fileSize);
		memset((void *)(virtAddr + fileSize * 8), 0, (memSize - fileSize) * 8);
	}
  }
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

