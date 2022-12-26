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
Context *kcontext(Area kstack, void (*entry)(void *), void *arg);
Context *ucontext(AddrSpace *as, Area kstack, void *entry);

static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  Elf_Ehdr ehdr;
  int fd = fs_open(filename, 0, 0);
  fs_lseek(fd, 0, SEEK_SET);
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
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read(fd, phdr, ehdr.e_phnum * sizeof(Elf_Phdr));
  for (size_t i = 0; i < ehdr.e_phnum; i++) {
	if (phdr[i].p_type == PT_LOAD) {
		size_t offset = phdr[i].p_offset;
		size_t virtAddr = phdr[i].p_vaddr;
		size_t fileSize = phdr[i].p_filesz;
		size_t memSize = phdr[i].p_memsz;
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

void context_kload(PCB *pcb, void (*entry)(void *), void *arg)
{
	Area kstack;
	kstack.start = pcb->stack;
	kstack.end = kstack.start + sizeof(pcb->stack);
	pcb->cp = kcontext(kstack, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[])
{
	Area ustack;
	ustack.start = heap.end - sizeof(pcb->stack);
	ustack.end = heap.end;
	pcb->cp = ucontext(NULL, ustack, (void *)loader(pcb, filename));

	//initializing argc, argv and envp.

	//get the argc and sz_envp

	int i = 0;
	while (argv[i] != NULL) i++;
	int argc = i;
	i = 0;
	while (envp[i] != NULL) i++;
	int sz_envp = i;

	//create the String area

	uintptr_t stack_argv[argc], stack_envp[sz_envp];
	char *now = (char *)(ustack.end - 2);
	*now = 0;
	for (int j = sz_envp - 1; j >= 0; j--) {
		now -= strlen(envp[j]);
		stack_envp[j] = (uintptr_t)now;
		strcpy(now, envp[j]);
		printf("%s\n", now);
		now--; *now = 0;
		//printf("%p\n", now);
	}
	for (int j = argc - 1; j >= 0; j--) {
		now -= strlen(argv[j]);
		stack_argv[j] = (uintptr_t)now;
		strcpy(now, argv[j]);
		printf("%s\n", now);
		printf("%p\n", now);
		now--; *now = 0;
	}
	
	//store the envp, argv and argc

	uintptr_t *p = (uintptr_t *)now;
	p--; *p = 0;
	for (int j = sz_envp - 1; j >= 0; j--) {
		p--;
		*p = stack_envp[j];
		printf("%p\n", p);
	}
	p--; *p = 0;
	for (int j = argc - 1; j >= 0; j--) {
		p--;
		*p = stack_argv[j];
		printf("%p\n", p);
	}
	p--; *p = argc;
	
	//update the cp->gprx

	pcb->cp->GPRx = (uintptr_t)p;

}