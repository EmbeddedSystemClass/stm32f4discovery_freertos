

#include "isr.hpp"

static InterruptVectors interruptVectors;
InterruptVectors& GetInterruptVectorsInstance() {
  return interruptVectors;
}

extern "C" {

void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress) {
  /* These are volatile to try and prevent the compiler/linker optimising them
  away as the variables never actually get used.  If the debugger won't show the
  values of the variables, make them global my moving their declaration outside
  of this function. */
  volatile uint32_t r0 __attribute__((unused));
  volatile uint32_t r1 __attribute__((unused));
  volatile uint32_t r2 __attribute__((unused));
  volatile uint32_t r3 __attribute__((unused));
  volatile uint32_t r12 __attribute__((unused));
  volatile uint32_t lr __attribute__((unused)); /* Link register. */
  volatile uint32_t pc __attribute__((unused)); /* Program counter. */
  volatile uint32_t psr __attribute__((unused));/* Program status register. */

  r0 = pulFaultStackAddress[ 0 ];
  r1 = pulFaultStackAddress[ 1 ];
  r2 = pulFaultStackAddress[ 2 ];
  r3 = pulFaultStackAddress[ 3 ];

  r12 = pulFaultStackAddress[ 4 ];
  lr = pulFaultStackAddress[ 5 ];
  pc = pulFaultStackAddress[ 6 ];
  psr = pulFaultStackAddress[ 7 ];

  /* When the following line is hit, the variables contain the register values. */
  for ( ;; ) {}
}

void HardFault_Handler() {
  __asm volatile (
    // " tst, lr, #4                                               \n"
    // " ite eq                                                    \n"
    // " mrseq r0, msp                                             \n"
    // " mrsne r0, psp                                             \n"
    " ldr r1, [r0, #24]                                         \n"
    " bl prvGetRegistersFromStack    \n"
  );
}

void SPI1_IRQHandler() {
  interruptVectors.call<static_cast<uint8_t>(Callbacks::Spi1)>();
}

}
