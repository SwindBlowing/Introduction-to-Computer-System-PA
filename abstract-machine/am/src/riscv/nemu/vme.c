#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

#define VPN0(x) (((uintptr_t)(x) & 0x003ff000) >> 12)
#define VPN1(x) (((uintptr_t)(x) & 0xffc00000) >> 22)
#define PPN(x) (((uintptr_t)(x) & 0xfffffc00) >> 10)

void map(AddrSpace *as, void *va, void *pa, int prot) {
	va = (void *)((uintptr_t)(va) & (~0xfff));
	pa = (void *)((uintptr_t)(pa) & (~0xfff));

	PTE *PT_entry = as->ptr + VPN1(va) * 4;
	if (!(*PT_entry & 0x1)) {
		void *new_leaf_page = pgalloc_usr(PGSIZE);
		printf("newpage:%p\n", new_leaf_page);
		*PT_entry = (*PT_entry & 0x3ff) | (((uintptr_t)(new_leaf_page) >> 12) << 10);
		*PT_entry = (*PT_entry | 0x1);
		printf("%x\n", *PT_entry);
	}
	printf("%x\n", PPN(PT_entry));
	printf("%x\n", PPN(PT_entry) * 4098 + VPN0(va) * 4);
	PTE *leaf_PTE = (PTE *)(PPN(PT_entry) * 4098 + VPN0(va) * 4);
	*leaf_PTE = (0xfffffc00 & PPN(pa)) | 0xf;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {

  Context *cp = (Context *)kstack.end - 1;
  cp->mepc = (uintptr_t)entry;
  cp->mstatus = 0x1800;

  return cp;
}
