
cmake_minimum_required(VERSION 3.13)

include(${PICO_SDK_INIT_CMAKE_FILE})

project(pico_blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

project(mixplus-pico)

add_library(${PROJECT_NAME})
add_compile_options(-Og -g)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/spi_transmitonly.pio)

add_subdirectory(lib/FatFs_SPI fatfs)
add_subdirectory(lib/pico-mpu6050 mpu6050)

target_sources(${PROJECT_NAME} PUBLIC src/mixplus.cpp)

target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_dma hardware_spi hardware_pio hardware_adc hardware_i2c FatFs_SPI haw_mpu6050 hardware_pwm pico_multicore)
target_include_directories(${PROJECT_NAME} PUBLIC inc)