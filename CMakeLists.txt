# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
include(${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(${CMAKE_CURRENT_LIST_DIR}/pico-sdk/pico_sdk_init.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(samwise-adcs-motor C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

file(GLOB top CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB adm1176 CONFIGURE_DEPENDS "src/drivers/adm1176/*.cpp")
file(GLOB init CONFIGURE_DEPENDS "src/init/*.cpp")
file(GLOB watchdog CONFIGURE_DEPENDS "src/drivers/watchdog/*.cpp")
file(GLOB motor CONFIGURE_DEPENDS "src/drivers/motor/*.cpp")

add_executable(samwise-adcs-motor ${top} ${adm1176} ${init} ${watchdog} ${motor})

pico_set_program_name(samwise-adcs-motor "samwise_adcs_motor")
pico_set_program_version(samwise-adcs-motor "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(samwise-adcs-motor 0)
pico_enable_stdio_usb(samwise-adcs-motor 1)

# Add the standard library to the build
target_link_libraries(samwise-adcs-motor
        pico_stdlib 
		pico_util
		pico_time
		hardware_i2c
		hardware_pwm
		hardware_spi
		hardware_gpio)

# Add the standard include files to the build
target_include_directories(samwise-adcs-motor PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)
# The below was in there and I don't think I need or want it, but here it is
# ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required

target_include_directories(samwise-adcs-motor PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

pico_add_extra_outputs(samwise-adcs-motor)
