#
# Copyright (c) 2020, University of Southampton and Contributors.
# All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#


cmake_minimum_required(VERSION 3.1)

set(TARGET_NAME "temperature")
set(DEVICE "msp430fr5994")
set(USB_PORT usbmodem143301)

add_executable(
    ${TARGET_NAME}
    main.c
)

set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".elf")

target_include_directories(${TARGET_NAME} PRIVATE ${MSP430_GCC_DIR}/include)

target_compile_options(
    ${TARGET_NAME}
    PRIVATE -std=c99
    # PRIVATE -O2
    PRIVATE -g 
    PRIVATE -mmcu=${DEVICE} 
    PRIVATE -msmall
    # PRIVATE -mhwmult=none
    PRIVATE -mhwmult=f5series
    PRIVATE -Wall
    PRIVATE -DMSP430_ARCH
    PRIVATE -DNDEBUG
)

# linker
target_link_directories(
    ${TARGET_NAME} 
    PRIVATE ${MSP430_GCC_DIR}/include
    PRIVATE ${MSP430_GCC_DIR}/msp430-elf/lib
    PRIVATE ${MSP430_GCC_DIR}/lib/gcc/msp430-elf/9.2.0
)
target_link_options(
    ${TARGET_NAME}
    PRIVATE -T ${MSP430_GCC_DIR}/include/${DEVICE}.ld
    PRIVATE -T ${MSP430_GCC_DIR}/include/${DEVICE}_symbols.ld
)

add_upload(${TARGET_NAME})