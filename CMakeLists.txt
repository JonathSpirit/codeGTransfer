cmake_minimum_required(VERSION 3.10)

include(ExternalProject)

#Project
project(codeGTransfer VERSION 0.1)

#Policy
cmake_policy(SET CMP0076 NEW) #target_sources

#Variables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)

#Default is 32bit
if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "32")
endif()

#Set cache entry
set(TARGET_ARCH "" CACHE STRING "Set architecture type (32 or 64)")

#Not defined = default to empty
if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "")
endif()

#Set compiler 32/64 flag
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

#Add external project
if (WIN32)
    ExternalProject_Add(catkin
        GIT_REPOSITORY  https://github.com/ros/catkin.git
        GIT_TAG         noetic-devel
        CMAKE_ARGS      -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/catkin
        INSTALL_COMMAND ""
    )
endif()

ExternalProject_Add(serial
    GIT_REPOSITORY  https://github.com/wjwwood/serial.git
    GIT_TAG         master
    CMAKE_ARGS      -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/serial
    if (WIN32)
        CMAKE_ARGS      -D catkin_DIR=${CMAKE_BINARY_DIR}/catkin-prefix/src/catkin-build/devel/share/catkin/cmake
        if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
            CMAKE_ARGS      -D CMAKE_CXX_FLAGS=${TARGET_ARCH_FLAG}
        endif()
    else()
        CMAKE_ARGS      -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/serial
        CMAKE_ARGS      -D CMAKE_CXX_FLAGS=${TARGET_ARCH_FLAG}
    endif()
    CMAKE_ARGS      -D CMAKE_BUILD_TYPE=Release
)

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

#Dependency
if (WIN32)
    add_dependencies(${PROJECT_NAME} catkin)
endif()

add_dependencies(${PROJECT_NAME} serial)

#Includes path
target_include_directories(${PROJECT_NAME} PUBLIC "include/")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}/serial/include/")

#Sources file
target_sources(${PROJECT_NAME} PUBLIC "src/main.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_string.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_checksum.cpp")

#Copy serial dll
if (WIN32)
    if (MSVC)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_BINARY_DIR}/serial/bin/serial.dll
                    ${CMAKE_BINARY_DIR}/serial.dll)
    else ()
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_BINARY_DIR}/serial/bin/libserial.dll
                    ${CMAKE_BINARY_DIR}/libserial.dll)
    endif ()
endif()

#Library
if (WIN32)
    if (MSVC)
        target_link_libraries(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}/serial/lib/serial.lib")
    else()
        target_link_libraries(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}/serial/lib/libserial.dll.a")
    endif()
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}/serial/lib/libserial.so")
endif()
