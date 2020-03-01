

#pragma once

#define ETL_VERBOSE_ERRORS
#define ETL_CHECK_PUSH_POP
#define ETL_ISTRING_REPAIR_ENABLE
#define ETL_IVECTOR_REPAIR_ENABLE
#define ETL_IDEQUE_REPAIR_ENABLE
#define ETL_CALLBACK_TIMER_USE_ATOMIC_LOCK
#define ETL_NO_STL

// #include "etl/profiles/armv7_no_stl.h"
#include "etl/profiles/auto.h"
// // ARM6 compiler
// #if defined(ETL_NO_STL)
// #include "etl/profiles/armv6_no_stl.h"
// #else
// #include "etl/profiles/armv6.h"
// #endif

