cmake_minimum_required(VERSION 3.10)

include(ExternalProject)
ExternalProject_Add(catkin
    GIT_REPOSITORY  https://github.com/ros/catkin.git
    GIT_TAG         noetic-devel
    CMAKE_ARGS      -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/catkin
    INSTALL_COMMAND ""
)

ExternalProject_Add(serial
    GIT_REPOSITORY  https://github.com/wjwwood/serial.git
    GIT_TAG         master
    CMAKE_ARGS      -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/serial
    CMAKE_ARGS      -D catkin_DIR=${CMAKE_BINARY_DIR}/catkin-prefix/src/catkin-build/devel/share/catkin/cmake
)

#Project
project(codeGTransfer VERSION 0.1)

#Enabling CTest
#enable_testing()

#Policy
cmake_policy(SET CMP0076 NEW) #target_sources

#Variables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)

#Copy example folder
#file(COPY "example/" DESTINATION "example/")



#Default is 32bit
if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "32")
endif()

#Set cache entry
set(TARGET_ARCH "32" CACHE STRING "Set architecture type (32 or 64)")

#Configure header file
configure_file(CMakeConfig.hpp.in CMakeConfig.hpp)

#C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Set compiler options
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using Clang
    if (TARGET_ARCH MATCHES "32")
        #32bit
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -m32")
    else()
        #64bit
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -m64")
    endif()
    
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # using Visual Studio C++
    set(CMAKE_CXX_FLAGS "/EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    # using GCC
    if (TARGET_ARCH MATCHES "32")
        #32bit
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -m32")
    else()
        #64bit
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -m64")
    endif()
    
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")
endif()

#Executable
add_executable(${PROJECT_NAME})

#Includes path
target_include_directories(${PROJECT_NAME} PUBLIC "include/")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}/serial/include/")

#Sources file
target_sources(${PROJECT_NAME} PUBLIC "src/main.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_string.cpp")

#Dependency
add_dependencies(${PROJECT_NAME} catkin serial)

#Copy serial dll
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/serial/bin/libserial.dll
                ${CMAKE_BINARY_DIR}/libserial.dll)
endif()

#Library
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}/serial/lib/libserial.dll.a")
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}/serial/lib/libserial.so")
endif()



#Add test
#add_test(NAME "CompilingTestFile" COMMAND ${PROJECT_NAME} "--in=example/test")
