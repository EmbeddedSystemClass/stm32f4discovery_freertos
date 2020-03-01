cmake_minimum_required(VERSION 3.6)
project (stm32f4 C CXX ASM)

set(ARM_GCC_PATH "/opt/gcc-arm-none-eabi-8.3.1/bin")
set(OPT_FLAGS "-g -O0")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
set(OBJCOPY "${ARM_GCC_PATH}/arm-none-eabi-objcopy")
set(SIZE "${ARM_GCC_PATH}/arm-none-eabi-size")
set(NM "${ARM_GCC_PATH}/arm-none-eabi-nm")

set(PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")
set(FREERTOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../modules/freertos")
set(SEGGER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../modules/segger")
set(STM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../modules/stm")
set(CM3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../modules/libopencm3")
set(MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../modules")
set(STARTUP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../startup")
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/stm32_flash.ld")

set(CORE_FLAGS "--specs=nosys.specs -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16")

set(CMAKE_C_FLAGS "${CORE_FLAGS} -fno-builtin -Wall -std=gnu99 -fdata-sections -ffunction-sections ${OPT_FLAGS}")
set(CMAKE_CXX_FLAGS "${CORE_FLAGS} -fno-rtti -fno-builtin -Wall -std=c++14 -fdata-sections -ffunction-sections ${OPT_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKER_SCRIPT}")

#file(GLOB USER_SRCS "src/*.cpp")
set(USER_SRCS "${PROJ_DIR}/src/cdc_acm.c")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/gpio.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/spi1.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/usb.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/cli.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/isr.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/spi.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/lis3dsh.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/led.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/assert_err.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/i2c.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/i2c1.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/cs43l22.cpp")
set(USER_SRCS ${USER_SRCS} "${PROJ_DIR}/src/main.cpp")
set(RTOS_SRCS "${FREERTOS_DIR}/list.c")
set(RTOS_SRCS ${RTOS_SRCS} "${FREERTOS_DIR}/FreeRTOS_CLI.c")
set(RTOS_SRCS ${RTOS_SRCS} "${FREERTOS_DIR}/queue.c")
set(RTOS_SRCS ${RTOS_SRCS} "${FREERTOS_DIR}/tasks.c")
set(RTOS_SRCS ${RTOS_SRCS} "${FREERTOS_DIR}/timers.c")
set(RTOS_SRCS ${RTOS_SRCS} "${FREERTOS_DIR}/heap_4.c")
set(RTOS_SRCS ${RTOS_SRCS} "${FREERTOS_DIR}/port.c")
set(RTOS_SRCS ${RTOS_SRCS} "${FREERTOS_DIR}/event_groups.c")
set(SEGGER_SRCS "${SEGGER_DIR}/SEGGER_SYSVIEW_Config_FreeRTOS.c")
set(SEGGER_SRCS ${SEGGER_SRCS} "${SEGGER_DIR}/SEGGER_SYSVIEW_FreeRTOS.c") 
set(SEGGER_SRCS ${SEGGER_SRCS} "${SEGGER_DIR}/SEGGER_RTT.c")
set(SEGGER_SRCS ${SEGGER_SRCS} "${SEGGER_DIR}/SEGGER_SYSVIEW.c")
file(GLOB STM_SRCS "${STM_DIR}/*.c")
file(GLOB CM3_SRCS "${CM3_DIR}/lib/cm3/*.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/flash_common_all.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/flash_common_f.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/flash_common_f24.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/flash_common_idcache.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/gpio_common_all.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/gpio_common_f0234.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/rcc_common_all.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/spi_common_all.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/spi_common_v1.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/common/i2c_common_v1.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/f4/flash.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/f4/gpio.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/f4/pwr.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/stm32/f4/rcc.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/usb/usb.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/usb/usb_control.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/usb/usb_dwc_common.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/usb/usb_f107.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/usb/usb_msc.c")
file(GLOB CM3_SRCS ${CM3_SRCS} "${CM3_DIR}/lib/usb/usb_standard.c")
set(STARTUP "${STARTUP_DIR}/startup_stm32f4xx.s")

include_directories(${PROJ_DIR}/inc
                    ${STM_DIR}
                    ${CM3_DIR}
                    ${CM3_DIR}/lib/usb
                    ${MODULES_DIR})

add_executable(${PROJECT_NAME}.elf ${RTOS_SRCS} ${USER_SRCS} ${SEGGER_SRCS} ${STM_SRCS} ${CM3_SRCS} ${STARTUP})


add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  # build .hex and .bin files
  COMMAND ${OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
  COMMAND ${OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
  COMMAND ${SIZE} --format=berkeley ${PROJECT_NAME}.elf
  COMMAND ${NM} --print-size --size-sort ${PROJECT_NAME}.elf > functions_by_size.txt
  COMMAND ${NM} --print-size --numeric-sort ${PROJECT_NAME}.elf > functions_by_addr.txt
  COMMENT "Building hex and bin files"
)

add_custom_target(flash
  COMMAND JLinkExe -if swd -device STM32F407VG -speed 4000 -autoconnect 1 -CommanderScript ${CMAKE_CURRENT_SOURCE_DIR}/program.jlink
  COMMENT "Flashing device"
)
