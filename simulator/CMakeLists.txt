cmake_minimum_required(VERSION 3.20)
project(DieselFireSimulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Platform-specific setup ────────────────────────────────────
if(EMSCRIPTEN)
  # Emscripten/WASM build — SDL2 via emscripten ports
  set(SDL2_INCLUDE_DIR "")
  set(SDL2_LIBRARY "")
  set(PLATFORM_LIBS m)
  set(PLATFORM_LINK_OPTS
    --use-port=sdl2
    -sALLOW_MEMORY_GROWTH
    -sMAX_WEBGL_VERSION=2
    -sFULL_ES2
  )
  set(PLATFORM_COMPILE_OPTS --use-port=sdl2)
else()
  # Native build (Linux)
  set(SDL2_INCLUDE_DIR "/tmp/opencode/SDL-release-2.32.4/include")
  set(SDL2_LIBRARY "-l:libSDL2-2.0.so.0")
  set(PLATFORM_LIBS m dl pthread)
  set(PLATFORM_LINK_OPTS "")
  set(PLATFORM_COMPILE_OPTS "")
endif()

# ── LVGL ───────────────────────────────────────────────────────
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

FetchContent_GetProperties(lvgl)
if(NOT lvgl_POPULATED)
  FetchContent_Populate(lvgl)
  set(HELIUM_FILE "${lvgl_SOURCE_DIR}/src/draw/sw/blend/helium/lv_blend_helium.S")
  if(EXISTS "${HELIUM_FILE}")
    file(REMOVE "${HELIUM_FILE}")
    message(STATUS "Patched LVGL: removed Helium ASM")
  endif()
  add_subdirectory(${lvgl_SOURCE_DIR} ${lvgl_BINARY_DIR})
endif()

set(PROJECT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Source file groups
set(SCREEN_SRC
  screens/DieselScreen.cpp screens/MainStatusScreen.cpp
  screens/SettingsMenuScreen.cpp screens/ThermostatScreen.cpp
  screens/FrostScreen.cpp screens/HumidityScreen.cpp
  screens/TimeoutsScreen.cpp screens/TimerChartScreen.cpp
  screens/TimerEditScreen.cpp screens/WiFiScreen.cpp
  screens/MQTTScreen.cpp screens/BTScreen.cpp
  screens/SensorScreen.cpp screens/GPIOScreen.cpp
  screens/FuelMixtureScreen.cpp screens/HeaterSettingsScreen.cpp
  screens/FuelCalScreen.cpp screens/LVCScreen.cpp
  screens/SystemScreen.cpp screens/RebootScreen.cpp
  screens/PasswordScreen.cpp screens/HourMeterScreen.cpp
  screens/InheritScreen.cpp screens/BWDebugScreen.cpp
)
set(MOCK_SRC
  mock/mock_globals.cpp mock/mock_data.cpp
  mock/mock_tempsense.cpp mock/mock_utilclasses.cpp
)
set(REAL_PROTOCOL_SRC
  ${PROJECT_SRC}/src/Protocol/Protocol.cpp
  ${PROJECT_SRC}/src/Utility/MODBUS-CRC16.cpp
)
set(MAIN_SRC main.cpp)
set(BLUWIRE_SRC vserial.cpp heater_emu.cpp bluwire_task.cpp)

# Include paths
set(INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PROJECT_SRC}/src
  ${lvgl_SOURCE_DIR}/src
  ${lvgl_SOURCE_DIR}
)
if(NOT EMSCRIPTEN)
  list(APPEND INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
endif()

# ── Main simulator target ──────────────────────────────────────
add_executable(dieselfire-sim
  ${MAIN_SRC} ${MOCK_SRC} ${SCREEN_SRC}
  ${BLUWIRE_SRC} ${REAL_PROTOCOL_SRC}
)

target_include_directories(dieselfire-sim PRIVATE ${INCLUDE_DIRS} screens)
target_include_directories(dieselfire-sim BEFORE PRIVATE ${PROJECT_SRC}/src/Protocol)
target_compile_options(dieselfire-sim PRIVATE
  ${PLATFORM_COMPILE_OPTS}
  "-include${CMAKE_CURRENT_SOURCE_DIR}/include/Arduino.h"
)
target_compile_definitions(dieselfire-sim PRIVATE
  LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h"
)
target_link_options(dieselfire-sim PRIVATE ${PLATFORM_LINK_OPTS})
target_link_libraries(dieselfire-sim PRIVATE lvgl::lvgl ${SDL2_LIBRARY} ${PLATFORM_LIBS})

if(EMSCRIPTEN)
  set_target_properties(dieselfire-sim PROPERTIES SUFFIX ".html")
endif()

# ── Headless test executable ───────────────────────────────────
add_executable(bluewire-test
  test_bluewire.cpp
  ${BLUWIRE_SRC} ${REAL_PROTOCOL_SRC}
  mock/mock_globals.cpp mock/mock_utilclasses.cpp
  mock/mock_data.cpp mock/mock_tempsense.cpp
)
target_include_directories(bluewire-test PRIVATE
  ${INCLUDE_DIRS} ${PROJECT_SRC}/src/Protocol
)
target_compile_options(bluewire-test PRIVATE
  "-include${CMAKE_CURRENT_SOURCE_DIR}/include/Arduino.h"
)
target_compile_definitions(bluewire-test PRIVATE BLUELINE_GLOBALS_EXTERNAL)
target_link_libraries(bluewire-test PRIVATE ${PLATFORM_LIBS})
