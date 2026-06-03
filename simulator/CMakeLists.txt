cmake_minimum_required(VERSION 3.20)
project(DieselFireSimulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 (use system library + source headers for dev files)
set(SDL2_INCLUDE_DIR "/tmp/opencode/SDL-release-2.32.4/include")
set(SDL2_LIBRARY "/usr/lib/x86_64-linux-gnu/libSDL2-2.0.so.0")

# LVGL
include(FetchContent)
FetchContent_Declare(
  lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG v9.5.0
)
set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE PATH "")
set(LV_BUILD_EXAMPLES OFF CACHE BOOL "")
set(LV_BUILD_DEMOS OFF CACHE BOOL "")
FetchContent_MakeAvailable(lvgl)

# Remove Helium SIMD assembly (ARM-only, breaks x86 build)
file(GLOB_RECURSE HELIUM_FILES "${lvgl_SOURCE_DIR}/src/draw/sw/blend/helium/*.S")
if(HELIUM_FILES)
  file(REMOVE ${HELIUM_FILES})
  message(STATUS "Removed ${HELIUM_FILES} Helium SIMD files (x86-64 host)")
endif()

set(PROJECT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Screen source files (simulator copies with modified includes)
set(SCREEN_SRC
  screens/DieselScreen.cpp
  screens/MainStatusScreen.cpp
  screens/SettingsMenuScreen.cpp
  screens/ThermostatScreen.cpp
  screens/FrostScreen.cpp
  screens/HumidityScreen.cpp
  screens/TimeoutsScreen.cpp
)

# Mock globals
set(MOCK_SRC
  mock/mock_globals.cpp
  mock/mock_data.cpp
)

# Main
set(MAIN_SRC
  main.cpp
)

# Include paths
set(INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PROJECT_SRC}/src
  ${lvgl_SOURCE_DIR}/src
  ${lvgl_SOURCE_DIR}
  ${SDL2_INCLUDE_DIR}
)

add_executable(dieselfire-sim
  ${MAIN_SRC}
  ${MOCK_SRC}
  ${SCREEN_SRC}
)

target_include_directories(dieselfire-sim PRIVATE ${INCLUDE_DIRS} screens)
target_link_libraries(dieselfire-sim PRIVATE lvgl::lvgl ${SDL2_LIBRARY} m dl pthread)
target_compile_definitions(dieselfire-sim PRIVATE LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")
