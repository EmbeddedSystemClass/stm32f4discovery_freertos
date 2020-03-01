

#pragma once

#include "stdint.h"
#include "stdbool.h"
#include "missing_cm3_incs.hpp"

// Allows the asserts to be removed for release build
#ifdef ASSERT_ENABLED

// Data that recorded during an assert (program counter, link register, line in code)
typedef struct {
  uint32_t pc;
  uint32_t lr;
  uint32_t line;
} assert_info_t;

extern assert_info_t assert_info;

// this is ARM and GCC specific syntax
#define GET_LR() __builtin_return_address(0);
#define GET_PC(_a) __asm volatile ("mov %0, pc" : "=r" (_a))

// Record the data to be used in custom assert function
#define ASSERT_RECORD          \
  do {                         \
    void *pc;                  \
    GET_PC(pc);                \
    void *lr = GET_LR();       \
    uint32_t line = __LINE__;  \
    assert_call(pc, lr, line); \
  } while (0)

// Custom assert used for return codes with 0 = SUCCESS and anything else is an error.
// Codes are enum classes which is why there is a cast.
//
// Input: An enum class that is returned from a function
#define ASSERT_RETURN(exp)        \
  do {                            \
    if (static_cast<bool>(exp)) { \
      __disable_irq();            \
      ASSERT_RECORD;              \
    }                             \
  } while (0)

// Custom assert used in place of normal assert (verifying if expression is true).
//
// Input: exp = Expression (i.e. *ptr != NULL)
#define ASSERT_ERR(exp) \
  do {                  \
    if (!(exp)) {       \
      __disable_irq();  \
      ASSERT_RECORD;    \
    }                   \
  } while (0)


// Assert function which stores program state and then either resets the program or sets a breakpoint
// to halt program if using a debugger.
//
// Input: pc = Program counter recorded during error
// Input: lr = Link register recorded during error
// Input: line = Line number in file
void assert_call(void* pc, void* lr, uint32_t line);

#else

// Defines are empty when asserts are not enabled to save on program size.
#define ASSERT_RETURN(exp)
#define ASSERT_ERR(exp)

#endif
