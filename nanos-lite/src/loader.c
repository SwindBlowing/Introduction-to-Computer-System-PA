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

void *loader_new_page(AddrSpace *as, size_t va, size_t sz)
{
	//printf("%x %x\n", va, va + sz - 1);
	size_t pageNum = ((va + sz - 1) >> 12) - (va >> 12) + 1;
	void *pa = new_page(pageNum);
	for (int i = 0; i < pageNum; i++) {
		//printf("%x\n", (va & ~0xfff) + i * PGSIZE);
		map(as, (void *)((va & ~0xfff) + i * PGSIZE),
			(void *)(pa + i * PGSIZE), 3);
	}
	return pa;
}

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
		void *phy_page_start = loader_new_page(&pcb->as, virtAddr, memSize);
		fs_lseek(fd, offset, SEEK_SET);
		//fs_read(fd, (void *)virtAddr, fileSize);
		fs_read(fd, phy_page_start + (virtAddr & 0xfff), fileSize);
		assert(memSize >= fileSize);
		//memset((void *)(virtAddr + fileSize), 0, memSize - fileSize);
		memset(phy_page_start + (virtAddr & 0xfff) + fileSize, 0, memSize - fileSize);
		/*if (fileSize < memSize) {
			pcb->max_brk = ROUNDUP(virtAddr + memSize, 0x1000);
		}*/
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
	ustack.end = ustack.start + PGSIZE;
	for (int i = 8; i; i--)
		map(&pcb->as, pcb->as.area.end - i * PGSIZE, ustack.end - i * PGSIZE, 3);
	pcb->cp = ucontext(&pcb->as, ustack, (void *)loader(pcb, filename));
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
	//In PA4.1, there is no need to actually realize the envp-String part.
	//So currently I just abandon it.
	//Don't forget it!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//create the String area

	uintptr_t stack_argv[argc], stack_envp[sz_envp];
	char *now = (char *)(pcb->cp - 4);
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

	pcb->cp->GPRx = (uintptr_t)p;
}