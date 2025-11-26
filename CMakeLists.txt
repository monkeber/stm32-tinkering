cmake_minimum_required(VERSION 4.0.0)

project(blinky C CXX ASM)

add_executable(${PROJECT_NAME} main.cpp)

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
    -fdata-sections
    -I.
    -mcpu=cortex-m4
    -mthumb
    -mfloat-abi=hard
    -mfpu=fpv4-sp-d16
)

target_link_options(${PROJECT_NAME} PRIVATE
    -T${CMAKE_SOURCE_DIR}/link.ld
    -nostartfiles
    -nostdlib
    --specs nano.specs
    -lc
    -lgcc
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
