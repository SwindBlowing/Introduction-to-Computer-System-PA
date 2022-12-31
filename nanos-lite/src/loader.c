#include <proc.h>
#include <elf.h>
#include <fs.h>
#include <memory.h>

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
void* new_page(size_t nr_page);
void protect(AddrSpace *as);
void map(AddrSpace *as, void *va, void *pa, int prot);


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
  
  pcb->max_brk = 0;

  Elf_Phdr phdr[ehdr.e_phnum];
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read(fd, phdr, ehdr.e_phnum * sizeof(Elf_Phdr));
  for (size_t i = 0; i < ehdr.e_phnum; i++) {
	if (phdr[i].p_type == PT_LOAD) {
		size_t offset = phdr[i].p_offset;
		size_t virtAddr = phdr[i].p_vaddr;
		size_t fileSize = phdr[i].p_filesz;
		size_t memSize = phdr[i].p_memsz;

		//printf("%x %x\n", virtAddr, virtAddr + memSize - 1);

		// new the pages.
		size_t pageNum = ((virtAddr + memSize - 1) >> 12) - (virtAddr >> 12) + 1;
		void *phyAddr = new_page(pageNum);
		for (int i = 0; i < pageNum; i++) {
			//printf("%x\n", (va & ~0xfff) + i * PGSIZE);
			map(&pcb->as, (void *)((virtAddr & ~0xfff) + i * PGSIZE),
				(void *)(phyAddr + i * PGSIZE), 3);
		}
		pcb->max_brk = pcb->max_brk > (virtAddr & ~0xfff) + pageNum * PGSIZE
						? pcb->max_brk : (virtAddr & ~0xfff) + pageNum * PGSIZE;
		//ends here

		size_t page_addr = virtAddr & 0xfff;
		fs_lseek(fd, offset, SEEK_SET);
		//fs_read(fd, (void *)virtAddr, fileSize);
		fs_read(fd, phyAddr + page_addr, fileSize);
		assert(memSize >= fileSize);
		//memset((void *)(virtAddr + fileSize), 0, memSize - fileSize);
		memset(phyAddr + page_addr + fileSize, 0, memSize - fileSize);
	}
  }
  fs_close(fd);
  //printf("entry of %s:%x\n", filename, ehdr.e_entry);
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
	//printf("kload entry:%p\n", entry);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[])
{
	protect(&pcb->as);

	Area ustack;
	ustack.start = new_page(8);
	ustack.end = ustack.start + PGSIZE * 8 - 4;
	for (int i = 8; i; i--)
		map(&pcb->as, pcb->as.area.end - i * PGSIZE, ustack.end - i * PGSIZE, 3);
	//pcb->cp = ucontext(&pcb->as, ustack, (void *)loader(pcb, filename));
	//initializing argc, argv and envp.

	//get the argc and sz_envp

	int i = 0;
	while (argv && argv[i] != NULL) i++;
	int argc = i;
	i = 0;
	//while (envp && envp[i] != NULL) i++;
	int sz_envp = i;
	// pay attention here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// there may not exist a "null" at the end of envp, so I don't know 
	// how to end the number-count.
	// In PA4.1, there is no need to actually realize the envp-String part.
	// So currently I just abandon it.
	// Don't forget it!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//create the String area

	uintptr_t stack_argv[argc], stack_envp[sz_envp];
	char *now = (char *)(ustack.end - 8);
	*now = 0;
	for (int j = sz_envp - 1; j >= 0; j--) {
		now -= strlen(envp[j]);
		stack_envp[j] = (uintptr_t)now;
		strcpy(now, envp[j]);
		now--; *now = 0;
	}
	for (int j = argc - 1; j >= 0; j--) {
		now -= strlen(argv[j]);
		stack_argv[j] = (uintptr_t)now;
		strcpy(now, argv[j]);
		now--; *now = 0;
	}

	while ((uintptr_t)now % 4) {
		now--; *now = 0;
	}
	
	//store the envp, argv and argc

	uintptr_t *p = (uintptr_t *)now;
	p--; *p = 0;
	for (int j = sz_envp - 1; j >= 0; j--) {
		p--;
		*p = stack_envp[j];
	}
	p--; *p = 0;
	for (int j = argc - 1; j >= 0; j--) {
		p--;
		*p = stack_argv[j];
	}
	p--; *p = argc;

	//ustack.end = (void *)p;
	Area kstack;
	kstack.start = &pcb->stack;
	kstack.end = kstack.start + sizeof(pcb->stack);
	pcb->cp = ucontext(&pcb->as, kstack, (void *)loader(pcb, filename));
	//ustack.end = ustack.start + PGSIZE * 8;

	pcb->cp->GPRx = (uintptr_t)p;
	//pcb->cp->gpr[2] = (uintptr_t)p;
	//printf("%x\n", (uintptr_t)p);
}