cmake_minimum_required(VERSION 3.14)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
project(NeoVACDM VERSION "1.3.3")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if (NOT PLUGIN_VERSION)
#    SET(PLUGIN_VERSION "custom-build")
#endif()
#add_compile_definitions(
#    PLUGIN_VERSION="${PLUGIN_VERSION}"
#)
#message(STATUS "Plugin version: ${PLUGIN_VERSION}")

configure_file(
    ${CMAKE_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_BINARY_DIR}/Version.h
)

# Add SDK include directory
include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/external/include
    ${CMAKE_SOURCE_DIR}/sdk
    ${CMAKE_SOURCE_DIR}/src
)

# Find external dependencies
find_package(CURL REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

# Source files
set(SOURCES
    src/config/ConfigParser.cpp
    src/core/DataManager.cpp
    src/core/Server.cpp
    src/log/Logger.cpp
    src/log/sqlite3.c
    src/log/sqlite3.h
    src/NeoVACDM.cpp
    src/main.cpp
)

# Define the plugin library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
    CURL::libcurl 
    JsonCpp::JsonCpp
)

# Set output directory and properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    PREFIX ""  # Remove "lib" prefix on Unix-like systems
)

# Install the plugin
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/plugins"
    LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/plugins"
)