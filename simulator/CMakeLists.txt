cmake_minimum_required(VERSION 3.20)
project(DieselFireSimulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 (use system library + source headers for dev files)
set(SDL2_INCLUDE_DIR "/tmp/opencode/SDL-release-2.32.4/include")
set(SDL2_LIBRARY "-l:libSDL2-2.0.so.0")

# LVGL
include(FetchContent)
set(LVGL_TAG v9.5.0)
FetchContent_Declare(
  lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG ${LVGL_TAG}
)
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE PATH "LVGL config file")
set(LV_BUILD_EXAMPLES OFF CACHE BOOL "")
set(LV_BUILD_DEMOS OFF CACHE BOOL "")

# Manually populate LVGL so we can patch out ARM Helium ASM before building
FetchContent_GetProperties(lvgl)
if(NOT lvgl_POPULATED)
  FetchContent_Populate(lvgl)

  # Remove ARM Helium SIMD assembly (incompatible with x86-64 assembler)
  set(HELIUM_FILE "${lvgl_SOURCE_DIR}/src/draw/sw/blend/helium/lv_blend_helium.S")
  if(EXISTS "${HELIUM_FILE}")
    file(REMOVE "${HELIUM_FILE}")
    message(STATUS "Patched LVGL: removed Helium ASM (${HELIUM_FILE})")
  endif()

  add_subdirectory(${lvgl_SOURCE_DIR} ${lvgl_BINARY_DIR})
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
  screens/TimerChartScreen.cpp
  screens/TimerEditScreen.cpp
  screens/WiFiScreen.cpp
  screens/MQTTScreen.cpp
  screens/BTScreen.cpp
  screens/SensorScreen.cpp
  screens/GPIOScreen.cpp
  screens/FuelMixtureScreen.cpp
  screens/HeaterSettingsScreen.cpp
  screens/FuelCalScreen.cpp
  screens/LVCScreen.cpp
)

# Mock globals
set(MOCK_SRC
  mock/mock_globals.cpp
  mock/mock_data.cpp
  mock/mock_tempsense.cpp
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
