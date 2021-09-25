cmake_minimum_required(VERSION 3.10)

#Project
project(codeGTransfer VERSION 0.2)

#Policy
cmake_policy(SET CMP0076 NEW) #target_sources

#Variables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)

#Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#Find submodules
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/submodules/serial/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! Please update submodules and try again.")
endif()

#Patch the serial directory with a new CMakeLists file (to avoid the catkin/python dependency until it's officialy removed in serial)
#https://github.com/wjwwood/serial
configure_file("${PROJECT_SOURCE_DIR}/submodules/CMakeLists_serial_nocatkin.txt" "${PROJECT_SOURCE_DIR}/submodules/serial/CMakeLists.txt" COPYONLY)

#Add serial
set(SERIAL_ENABLE_CATKIN OFF CACHE BOOL "enable/disable catkin in serial")
add_subdirectory("submodules/serial")

#Set cache entry
set(TARGET_ARCH "" CACHE STRING "Set architecture type (32 or 64 or nothing (auto))")

#Default TARGET_ARCH is nothing (auto)
if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "")
endif()

#Set compiler 32/64/(auto) flag
set(TARGET_ARCH_FLAG "")

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if (TARGET_ARCH MATCHES "32")
        set(TARGET_ARCH_FLAG "-m32")
    elseif (TARGET_ARCH MATCHES "64")
        set(TARGET_ARCH_FLAG "-m64")
    else()
        set(TARGET_ARCH_FLAG "")
    endif()
endif()

#Configure header file
configure_file(CMakeConfig.hpp.in CMakeConfig.hpp)

#C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Set compiler options
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using Clang
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${TARGET_ARCH_FLAG}")
    
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # using Visual Studio C++
    set(CMAKE_CXX_FLAGS "/EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    # using GCC
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${TARGET_ARCH_FLAG}")
    
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")
endif()

#Executable
add_executable(${PROJECT_NAME})

#Dependencies
add_dependencies(${PROJECT_NAME} serial)

#Includes path
target_include_directories(${PROJECT_NAME} PUBLIC "include/")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/submodules/serial/include/")

#Sources file
target_sources(${PROJECT_NAME} PUBLIC "src/main.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_string.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_checksum.cpp")

#Library
target_link_libraries(${PROJECT_NAME} serial)
