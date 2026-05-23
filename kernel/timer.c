/* Timer hook placeholder. ToyOS currently demonstrates cooperative yield. */
#include "types.h"
#include "defs.h"

void timer_init(void) {
  /*
   * The syscall path already handles timer trap classification. For a compact
   * teaching kernel we leave preemption disabled and use SYS_yield in demos.
   */
}
