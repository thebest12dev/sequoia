cmake_minimum_required (VERSION 3.10)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("sequoia")

# Get libnbt++ (custom fork)
include(FetchContent)
FetchContent_Declare(
  libnbtplusplus
  GIT_REPOSITORY https://github.com/thebest12dev/libnbtplusplus.git
  GIT_TAG master
)
find_package(ZLIB REQUIRED)
FetchContent_MakeAvailable(libnbtplusplus)
include_directories("${libnbtplusplus_BINARY_DIR}")
include_directories("${ZLIB_INCLUDE_DIR}")
# Sequoia source

file(GLOB_RECURSE SEQUOIA_SRC "${CMAKE_SOURCE_DIR}/src/core/*.cpp" "${CMAKE_SOURCE_DIR}/src/core/*.h")
list(FILTER SEQUOIA_SRC EXCLUDE REGEX ".*/Main\\.(cpp|h)$")
message(STATUS ${SEQUOIA_SRC})

add_library(sequoia STATIC ${SEQUOIA_SRC})
add_executable(sequoiacli "src/core/Main.cpp")
target_include_directories(sequoia PRIVATE "${libnbtplusplus_SOURCE_DIR}/include")

target_include_directories(sequoiacli PRIVATE "${libnbtplusplus_SOURCE_DIR}/include")

target_link_libraries(sequoia PRIVATE nbt++ ZLIB::ZLIB)
target_link_libraries(sequoiacli PRIVATE sequoia)


# some config and other stuff
include(GenerateExportHeader)
generate_export_header(sequoia
    BASE_NAME Sequoia
    EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/generated/SequoiaExport.h"
)
include_directories(sequoia PRIVATE "${CMAKE_BINARY_DIR}/generated/")

# C++20
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sequoia PROPERTY CXX_STANDARD 20)
  set_property(TARGET sequoiacli PROPERTY CXX_STANDARD 20)
endif()

