cmake_minimum_required(VERSION 3.25)
project(cs202-dungeon-game VERSION 1.0.0 LANGUAGES CXX)

# Require C++20 for SFML 3 and modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure SFML search path
# The system SFML is installed at C:/Program Files/SFML/SFML-3.0.2
set(SFML_ROOT "C:/Program Files/SFML/SFML-3.0.2")
list(APPEND CMAKE_PREFIX_PATH "${SFML_ROOT}")

# Find SFML 3 libraries
find_package(SFML 3 COMPONENTS Graphics Window System Audio Network REQUIRED)

# Integrate nlohmann/json using FetchContent
include(FetchContent)
set(CMAKE_TLS_VERIFY OFF)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Define the game executable target named 'game'
add_executable(game
    src/main.cpp

    # core
    src/core/game.cpp
    src/core/state-manager.cpp
    src/core/game-state.cpp
    src/core/states/main-menu-state.cpp

    # global-settings
    src/global-settings/setting-manager.cpp
    src/global-settings/color-palette-manager.cpp
)

# Link SFML and JSON libraries
target_link_libraries(game PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    SFML::Network
    nlohmann_json::nlohmann_json
)

# Set runtime output directory directly to build/ (instead of build/bin/)
set_target_properties(game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Automatically copy SFML DLLs to the build directory on Windows
if(WIN32)
    file(GLOB SFML_DLLS "${SFML_ROOT}/bin/*.dll")
    foreach(DLL ${SFML_DLLS})
        add_custom_command(TARGET game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}"
            "$<TARGET_FILE_DIR:game>"
            COMMENT "Copying SFML DLL: ${DLL}"
        )
    endforeach()
endif()
