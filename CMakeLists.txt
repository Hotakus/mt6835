
# this file is use to normal Cmake project

cmake_minimum_required(VERSION 3.16)
project(mt6835
        LANGUAGES C
        VERSION 1.0.0.0
        DESCRIPTION "mt6835-framework"
        HOMEPAGE_URL https://github.com/Hotakus/octo-foc
)
set(CMAKE_C_STANDARD 99)

configure_file(${PROJECT_SOURCE_DIR}/version.h.in ${PROJECT_SOURCE_DIR}/version.h)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections -ffunction-sections -Wall")

# main add
include_directories(./
                    ./src
)

file(GLOB_RECURSE SOURCES
     "./src/*.*"
)

add_library(mt6835 STATIC ${SOURCES})

target_include_directories(mt6835 PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/
)
