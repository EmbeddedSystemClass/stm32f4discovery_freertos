include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(ARM_GCC_PATH "/opt/gcc-arm-none-eabi-8.3.1/bin")

set(CMAKE_C_COMPILER "${ARM_GCC_PATH}/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${ARM_GCC_PATH}/arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "${ARM_GCC_PATH}/arm-none-eabi-gcc")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(ASM_OPTIONS "-x assembler-with-cpp")
set(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}")

add_definitions(-DSTM32F4
                -DASSERT_ENABLED
               )

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -Wl --gc-sections")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -Wl,-Map=freertos_nrf52.map,--gc-sections")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS}  -Wl,-Map=stm32f4.map,--gc-sections,--demangle")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS}  -Wl,-Map=stm32f4.map,--gc-sections")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
