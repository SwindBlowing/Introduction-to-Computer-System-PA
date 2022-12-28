#include <memory.h>
#include <proc.h>

void map(AddrSpace *as, void *va, void *pa, int prot);

static void *pf = NULL;

void* new_page(size_t nr_page) {
  pf += (nr_page * PGSIZE);
  return pf - nr_page * PGSIZE;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  //return NULL;
  void *start = new_page(n / PGSIZE);
  memset(start, 0, n);
  return start;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

extern PCB *current;
/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  uintptr_t max_pg_end = current->max_brk;
  intptr_t max_pg = ((intptr_t)(max_pg_end >> 12)) - 1;
  intptr_t brk_pg = (brk >> 12);
  printf("%x %x\n", max_pg_end, brk);
  //printf("Here\n");
  if (brk_pg > max_pg) {
	
	void *start = pg_alloc((brk_pg - max_pg) * PGSIZE);
	for (int i = 0; i < brk_pg - max_pg; i++) {
		//printf("%p %p\n", max_pg_end + i * PGSIZE, start + i * PGSIZE);
		map(&current->as, (void *)(max_pg_end + i * PGSIZE),
			start + i * PGSIZE, 3);
	}
	current->max_brk = (brk_pg + 1) * PGSIZE;
  }
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
