cmake_minimum_required(VERSION 3.13)

# note: this must happen before project()
include(pico_sdk_import.cmake)

project(usb_keyboard_firmware)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

# add_keyboard_target(<target-name> <KEYBOARD_DEFINE> [EXTRA_DEFINE ...])
function(add_keyboard_target NAME)
    add_executable(${NAME}
        src/main.c
        src/usb_keyboard.c
        src/usb_descriptors.c
        src/matrix.c
        src/keyboard.c
        src/taphold.c
        src/doubletap.c
        src/combo.c
        src/layers.c
        src/macro.c
        src/ll_alloc.c
        src/ws2812.c
        src/color.c
        src/leds.c
        src/mouse.c
        src/kb_config.c
        src/log.c

        # Specific keyboards
        src/machines/hex2a/machine.c
        src/machines/split2040/machine.c
        )

    target_compile_definitions(${NAME} PRIVATE ${ARGN})

    pico_enable_stdio_uart(${NAME} 0)
    pico_enable_stdio_usb(${NAME} 0)

    pico_set_linker_script(${NAME} ${CMAKE_CURRENT_LIST_DIR}/linkerscript.ld)

    target_link_libraries(${NAME} PRIVATE
        pico_stdlib
        hardware_resets
        hardware_irq
        hardware_pio
        hardware_flash
        )

    pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR generated)

    pico_add_extra_outputs(${NAME})
endfunction()

# Keyboards
add_keyboard_target(hex2a       KEYBOARD_HEX2A)
add_keyboard_target(split2040   KEYBOARD_SPLIT2040)
