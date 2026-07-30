# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/natha/Documents/PlatformIO/Projects/Together/build/_deps/arduino_emulator-src"
  "C:/Users/natha/Documents/PlatformIO/Projects/Together/build/_deps/arduino_emulator-build"
  "C:/Users/natha/Documents/PlatformIO/Projects/Together/build/_deps/arduino_emulator-subbuild/arduino_emulator-populate-prefix"
  "C:/Users/natha/Documents/PlatformIO/Projects/Together/build/_deps/arduino_emulator-subbuild/arduino_emulator-populate-prefix/tmp"
  "C:/Users/natha/Documents/PlatformIO/Projects/Together/build/_deps/arduino_emulator-subbuild/arduino_emulator-populate-prefix/src/arduino_emulator-populate-stamp"
  "C:/Users/natha/Documents/PlatformIO/Projects/Together/build/_deps/arduino_emulator-subbuild/arduino_emulator-populate-prefix/src"
  "C:/Users/natha/Documents/PlatformIO/Projects/Together/build/_deps/arduino_emulator-subbuild/arduino_emulator-populate-prefix/src/arduino_emulator-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/natha/Documents/PlatformIO/Projects/Together/build/_deps/arduino_emulator-subbuild/arduino_emulator-populate-prefix/src/arduino_emulator-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/natha/Documents/PlatformIO/Projects/Together/build/_deps/arduino_emulator-subbuild/arduino_emulator-populate-prefix/src/arduino_emulator-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
