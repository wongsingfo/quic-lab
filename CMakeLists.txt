cmake_minimum_required(VERSION 3.1)
project(quic-lab)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE off)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

# -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl
# -DOPENSSL_LIBRARIES=/usr/local/opt/openssl/lib
find_package(OpenSSL REQUIRED)
message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")

include_directories(${OPENSSL_INCLUDE_DIR})

# quic lab
include_directories(.)
add_subdirectory(crypto)
add_subdirectory(util)
add_subdirectory(transport)

set(test_source
        crypto/hkdf_test.cc
        util/string_test.cc
        transport/packet_test.cc)


# set up gtest
include(GoogleTest)
enable_testing()
add_executable(tests tests.cpp ${test_source})
target_link_libraries(tests gtest_main crypto transport util OpenSSL::Crypto)
gtest_discover_tests(tests)


