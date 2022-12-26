#include <memory.h>

static void *pf = NULL;
size_t single_page_size = ((size_t)1 << 15);

void* new_page(size_t nr_page) {
  pf += (nr_page * single_page_size);
  return pf - nr_page * single_page_size;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  return NULL;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
