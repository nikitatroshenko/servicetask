cmake_minimum_required(VERSION 3.8)
project(servicetask)

set(CMAKE_C_STANDARD 89)

set(SOURCE_FILES main.c config.c err_handling.c ini.c)
set(EXECUTABLE_NAME servicetask)
add_definitions(-DEXECUTABLE_NAME="${EXECUTABLE_NAME}")
add_executable(servicetask ${SOURCE_FILES})

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    link_directories(${OPENSSL_LIBRARIES})
    message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
    target_link_libraries(servicetask ${OPENSSL_LIBRARIES})
endif()

# INIH configuration
add_definitions(-DINI_USE_STACK=0)