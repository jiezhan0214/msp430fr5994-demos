#
# Copyright (c) 2021, University of Southampton and Contributors.
# All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#


cmake_minimum_required(VERSION 3.1)

set(TARGET_NAME "rx_fr5994")
set(DEVICE "msp430fr5994")
set(USB_PORT usbmodem142301)

add_executable(
    ${TARGET_NAME}
    main.c
    hal_spi_rf.c
    hal_spi_rf.h
    msp_nrf24.c
    msp_nrf24.h
    nRF24L01.h
)

set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".elf")

target_include_directories(${TARGET_NAME} PRIVATE ${MSP430_GCC_DIR}/include)

target_compile_options(
    ${TARGET_NAME}
    PRIVATE -DMSP430_ARCH
    PRIVATE -std=c99
    PRIVATE -mmcu=${DEVICE} 
    # PRIVATE -O2
    PRIVATE -msmall
    PRIVATE -g 
    PRIVATE -mhwmult=f5series
    # PRIVATE -fno-common
    PRIVATE -Wall
    # PRIVATE -fno-zero-initialized-in-bss # We don't want to zero out whole bss on every boot
    # PRIVATE -DNDEBUG
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
    # PRIVATE -T ${CMAKE_CURRENT_LIST_DIR}/../${DEVICE}.ld
    PRIVATE -T ${MSP430_GCC_DIR}/include/${DEVICE}_symbols.ld
    # PRIVATE -nostdlib
    # PRIVATE -ffreestanding
    # PRIVATE -nostartfiles
)

add_upload(${TARGET_NAME})