


#include "assert_err.hpp"

#ifdef ASSERT_ENABLED

assert_info_t assert_info;

void assert_call(void* pc, void* lr, uint32_t line) {
  assert_info.pc = (uint32_t)pc;
  assert_info.lr = (uint32_t)lr;
  assert_info.line = line;
  /* C_DEBUGEN == 1 -> Debugger Connected */
  if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
    /* Generate breakpoint if debugger is connected */
    __asm("bkpt 5");
  }
  NVIC_SystemReset();
}

#endif
