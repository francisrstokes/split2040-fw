cmake_minimum_required(VERSION 3.13)

# Keyboard to build:
add_definitions(-DKEYBOARD_HEX2A)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(usb_keyboard_firmware)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(usb_keyboard
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

        # Specific keyboards
        src/machines/hex2a/machine.c
        )

pico_enable_stdio_uart(usb_keyboard 0)
pico_enable_stdio_usb(usb_keyboard 0)

target_link_libraries(usb_keyboard PRIVATE pico_stdlib hardware_resets hardware_irq hardware_pio)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(usb_keyboard ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR generated)

pico_add_extra_outputs(usb_keyboard)
