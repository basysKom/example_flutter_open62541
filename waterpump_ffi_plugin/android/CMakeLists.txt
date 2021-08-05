cmake_minimum_required(VERSION 3.4.1)  # for example

add_subdirectory("../native_code/open62541" open62541)

add_library(waterpump_ffi_lib SHARED
    "../native_code/waterpump_ffi_lib.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native_code/sdk/runtime/include/dart_api_dl.c")

target_include_directories(waterpump_ffi_lib PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/../native_code/open62541/build/install_dir/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/../native_code/sdk/runtime")

target_link_libraries(waterpump_ffi_lib PRIVATE open62541)
