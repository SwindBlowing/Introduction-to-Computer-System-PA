#include <common.h>

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
	case EVENT_YIELD: printf("Received yield message!\n"); 
		printf("%x %x %x\n", c->mstatus, c->mepc, c->mcause); break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
