

#pragma once


#include "etl/delegate_service.h"

enum class Callbacks {
  Spi1,
  TotalCount
};

constexpr uint8_t LAST_INTERRUPT_NUM = static_cast<uint8_t>(Callbacks::TotalCount);
constexpr uint8_t INTERRUPT_OFFSET = 0;

typedef etl::delegate_service<LAST_INTERRUPT_NUM, INTERRUPT_OFFSET> InterruptVectors;

InterruptVectors& GetInterruptVectorsInstance();
