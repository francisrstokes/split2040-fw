cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(usb_keyboard_firmware)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(usb_keyboard
        src/usb_keyboard.c
        src/matrix.c
        src/keyboard.c
        src/taphold.c
        src/doubletap.c
        src/combo.c
        src/ll_alloc.c
        )

target_link_libraries(usb_keyboard PRIVATE pico_stdlib hardware_resets hardware_irq)
pico_add_extra_outputs(usb_keyboard)
