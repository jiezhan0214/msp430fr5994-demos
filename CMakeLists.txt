cmake_minimum_required(VERSION 3.1)

# configs
set(MSP430_GCC_DIR "/opt/msp430-gcc-9.2.0")

# set toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain.cmake)

project(MSP430FR5994-Test)
enable_language(C ASM)

add_subdirectory(tx_fr5994)
add_subdirectory(tx_fr5994_temp)
add_subdirectory(rx_fr6989)
add_subdirectory(temperature)
add_subdirectory(aes_temp_profile)
add_subdirectory(aes_profile)