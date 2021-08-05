cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "waterpump_ffi_plugin")
# Need to add C here in order to build everything properly
project(${PROJECT_NAME} LANGUAGES CXX C)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "waterpump_ffi_plugin_plugin")

add_subdirectory("../native_code/open62541" open62541)

add_library(${PLUGIN_NAME} SHARED
  "waterpump_ffi_plugin.cc"
  "../native_code/sdk/runtime/include/dart_api_dl.c"
  "../native_code/waterpump_ffi_lib.cpp"
)
apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter)
target_link_libraries(${PLUGIN_NAME} PRIVATE PkgConfig::GTK)

target_include_directories(${PLUGIN_NAME} PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/../native_code/open62541/build/install_dir/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/../native_code/sdk/runtime")

target_link_libraries(${PLUGIN_NAME} PRIVATE open62541)

# List of absolute paths to libraries that should be bundled with the plugin
set(waterpump_ffi_plugin_bundled_libraries
  ""
  PARENT_SCOPE
)
