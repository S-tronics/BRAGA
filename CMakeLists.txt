# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# This example uses an extra component from common functions such as WiFi and Ethernet connection.
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)
project(ATC-BRAGA-Componentized)

target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "components/AppOTAComponent/cert.pem" TEXT)