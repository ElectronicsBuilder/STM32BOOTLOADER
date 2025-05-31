cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME STM32Bootloader)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)




# ---------------------------
# ADD THIS: Custom sources
# ---------------------------



file(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_SOURCE_DIR}/Core/Src/*.c"
    "${CMAKE_SOURCE_DIR}/Core/Src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/drivers_test/**/*.cpp"
    "${CMAKE_SOURCE_DIR}/drivers_test/*.cpp"
    "${CMAKE_SOURCE_DIR}/UserDrivers/**/*.cpp"
    "${CMAKE_SOURCE_DIR}/UserDrivers/**/*.c"
    "${CMAKE_SOURCE_DIR}/logs/*.cpp"
    "${CMAKE_SOURCE_DIR}/logs/*.c"
)

message(STATUS "PROJECT_SOURCES = ${PROJECT_SOURCES}")

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCES}
)



# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/drivers
    ${CMAKE_SOURCE_DIR}/drivers_test
    ${CMAKE_SOURCE_DIR}/logs
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include

    ${CMAKE_SOURCE_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
    

    ${CMAKE_SOURCE_DIR}/UserDrivers/uart
    ${CMAKE_SOURCE_DIR}/UserDrivers/spi_flash
    #${CMAKE_SOURCE_DIR}/UserDrivers/nvram
    ${CMAKE_SOURCE_DIR}/UserDrivers/qspi_flash

    ${CMAKE_SOURCE_DIR}/UserDrivers/bootloader
    ${CMAKE_SOURCE_DIR}/UserDrivers/boot_defs
    ${CMAKE_SOURCE_DIR}/UserDrivers/fuse
    ${CMAKE_SOURCE_DIR}/UserDrivers/boot_transport   


)

target_include_directories(STM32Bootloader
    PRIVATE
    UserDrivers/components/w25q128
)

   # ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/DSP/Include  # <- Add this for arm_math.h
   # ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/NN/Include  # <- Add this for arm_math.h

   




# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

target_link_options(STM32Bootloader PRIVATE -u _printf_float)
