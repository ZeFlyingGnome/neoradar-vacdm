CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
SET(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
SET(VCPKG_TARGET_TRIPLET "x86-windows-static" CACHE STRING "")
    add_compile_definitions(DIR_SEPARATOR="\\\\")
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    # Force g++14 as g++13 does not implement some C++20 features
    set(CMAKE_CXX_COMPILER "g++-14")
    add_compile_definitions(DIR_SEPARATOR="/")
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    add_compile_definitions(DIR_SEPARATOR="/")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

PROJECT(NeoVACDM VERSION "1.4.0")
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# Use static runtime libraries
SET(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

IF (MSVC)
    IF (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        STRING(REGEX REPLACE "/W[0-4]" "/W4 /wd4100" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    ELSE ()
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    ENDIF ()
    IF (NOT CMAKE_CXX_FLAGS MATCHES "/MP")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP")
    ENDIF ()

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /sdl /permissive- /DNOMINMAX")
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /sdl /permissive- /DNOMINMAX")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFESTUAC:NO /ignore:4099")
    ADD_DEFINITIONS(/D_USRDLL)

    FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH()
ENDIF ()

if(DEV_BUILD)
    ADD_DEFINITIONS(-DDEV_BUILD)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-DDEBUG_BUILD=1) # enables log output to console window
    ADD_DEFINITIONS(-DDEV_BUILD)
endif()

CONFIGURE_FILE(
    ${CMAKE_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_BINARY_DIR}/Version.h
)

# No warnings for external libraries (EuroScopePlugIn.h)
INCLUDE_DIRECTORIES(
    SYSTEM ${CMAKE_SOURCE_DIR}/external/include
)

INCLUDE_DIRECTORIES(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/sdk
    ${CMAKE_SOURCE_DIR}/src
)

ADD_DEFINITIONS(
    -D_CRT_SECURE_NO_WARNINGS
    -DWIN32_LEAN_AND_MEAN
)

set(SOURCE_FILES
    src/config/ConfigParser.cpp
    src/core/DataManager.cpp
    src/core/Server.cpp
    src/log/SpdLogger.cpp
    src/NeoVACDM.cpp
    src/main.cpp
)


FIND_PACKAGE(OpenSSL CONFIG REQUIRED)
FIND_PACKAGE(httplib CONFIG REQUIRED)
FIND_PACKAGE(nlohmann_json CONFIG REQUIRED)
FIND_PACKAGE(spdlog CONFIG REQUIRED)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE
    OpenSSL::SSL OpenSSL::Crypto
    httplib::httplib
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

# move config file to output dir, allows loading of DLL from output dir
file(COPY ${CMAKE_SOURCE_DIR}/src/config/vacdm.txt DESTINATION ${CMAKE_BINARY_DIR})