cmake_minimum_required(VERSION 4.0.0)

project(blinky C CXX ASM)

include(FetchContent)

FetchContent_Declare(
  cmsis_f1
  GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis-device-f1.git
)

FetchContent_Declare(
  cmsis_core
  GIT_REPOSITORY https://github.com/ARM-software/CMSIS_5.git
  GIT_TAG 5.9.0
)

FetchContent_MakeAvailable(cmsis_f1 cmsis_core)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/startup.cpp
    src/syscalls.cpp
    src/hal.hpp
    src/hal.cpp
    ${cmsis_f1_SOURCE_DIR}/Source/Templates/gcc/startup_stm32f103xb.s
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
        /usr/arm-none-eabi/include
        ${cmsis_f1_SOURCE_DIR}/Include
        ${cmsis_core_SOURCE_DIR}/CMSIS/Core/Include

)

target_compile_options(${PROJECT_NAME} PRIVATE
    -W
    -Wall
    -Wextra
    -Werror
    -Wundef
    -Wshadow
    -Wdouble-promotion
    -Wformat-truncation
    -fno-common
    -Wconversion
    -g3
    -ffunction-sections
    -fno-exceptions
    -fdata-sections
    -mcpu=cortex-m3
    -mthumb
)

target_link_options(${PROJECT_NAME} PRIVATE
    -T${CMAKE_SOURCE_DIR}/src/link.ld
    -nostartfiles
    --specs=nano.specs
    -lc
    -lm
    -lgcc
    -mcpu=cortex-m3
    -mthumb
    -Wl,--gc-sections
    -Wl,-Map=${PROJECT_NAME}.map
)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
    ARGS -O binary $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Generating binary file from ELF"
)

# Custom target for flashing.

add_custom_target(flash
    COMMAND
          st-flash --reset write ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin 0x8000000
    COMMENT "flashes it"
)
add_dependencies(flash ${PROJECT_NAME})
