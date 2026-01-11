cmake_minimum_required(VERSION 4.0.0)

project(blinky C CXX ASM)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/syscalls.cpp
    src/hal.hpp
    src/hal.cpp
    ${CMAKE_SOURCE_DIR}/extern/cmsis_f1/Source/Templates/gcc/startup_stm32f103xb.s
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
        /usr/arm-none-eabi/include
        ${CMAKE_SOURCE_DIR}/extern/cmsis_f1/Include
        ${CMAKE_SOURCE_DIR}/extern/cmsis_core/CMSIS/Core/Include
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
