# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/anh/ProgramFile/esp32-idf/esp-idf/components/bootloader/subproject"
  "/home/anh/Projects/CODE/ESP_IDF/GatewayPlus_ESP32C3/build/bootloader"
  "/home/anh/Projects/CODE/ESP_IDF/GatewayPlus_ESP32C3/build/bootloader-prefix"
  "/home/anh/Projects/CODE/ESP_IDF/GatewayPlus_ESP32C3/build/bootloader-prefix/tmp"
  "/home/anh/Projects/CODE/ESP_IDF/GatewayPlus_ESP32C3/build/bootloader-prefix/src/bootloader-stamp"
  "/home/anh/Projects/CODE/ESP_IDF/GatewayPlus_ESP32C3/build/bootloader-prefix/src"
  "/home/anh/Projects/CODE/ESP_IDF/GatewayPlus_ESP32C3/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/anh/Projects/CODE/ESP_IDF/GatewayPlus_ESP32C3/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/anh/Projects/CODE/ESP_IDF/GatewayPlus_ESP32C3/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
