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
find_package(zlib-ng CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
FetchContent_MakeAvailable(libnbtplusplus)
include_directories("${libnbtplusplus_BINARY_DIR}")
include_directories("${ZLIB_INCLUDE_DIR}")
# Sequoia source

file(GLOB_RECURSE SEQUOIA_SRC "${CMAKE_SOURCE_DIR}/src/core/*.cpp" "${CMAKE_SOURCE_DIR}/src/core/*.h")
list(FILTER SEQUOIA_SRC EXCLUDE REGEX ".*/Main\\.(cpp|h)$")
list(FILTER SEQUOIA_SRC EXCLUDE REGEX ".*/CoreGui\\.(cpp|h)$")
message(STATUS ${SEQUOIA_SRC})

add_library(sequoia STATIC ${SEQUOIA_SRC}   "src/core/ExternalServer.h")
add_executable(sequoiacli "src/core/Main.cpp"

external/imgui/imgui.cpp
external/imgui/imgui_draw.cpp
external/imgui/imgui_tables.cpp
external/imgui/imgui_widgets.cpp
external/imgui/backends/imgui_impl_win32.cpp
external/imgui/backends/imgui_impl_dx11.cpp
"src/core/CoreGui.cpp"

  "src/core/ExternalServer.h")
target_include_directories(sequoia PRIVATE "${libnbtplusplus_SOURCE_DIR}/include")

target_include_directories(sequoiacli PRIVATE "${libnbtplusplus_SOURCE_DIR}/include")
target_include_directories(sequoiacli PRIVATE "${CMAKE_SOURCE_DIR}/external/imgui/")
target_link_libraries(sequoia PRIVATE nbt++ zlib-ng::zlib libzip::zip)
target_link_libraries(sequoiacli PRIVATE sequoia)


# some config and other stuff
include(GenerateExportHeader)
generate_export_header(sequoia
    BASE_NAME Sequoia
    EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/generated/SequoiaExport.h"
)
include_directories(sequoia PRIVATE "${CMAKE_BINARY_DIR}/generated/")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# C++20
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sequoia PROPERTY CXX_STANDARD 20)
  set_property(TARGET sequoiacli PROPERTY CXX_STANDARD 20)
endif()

