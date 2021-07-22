cmake_minimum_required (VERSION 3.2)
project (syncwatchUI)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE Debug)

# source files for the main executable
file(GLOB MY_SOURCES "src/*.cpp" "src/*/*.cpp")
file(GLOB IMGUI_SOURCES "src/imgui/*.cpp")
add_executable(syncwatchUI ${MY_SOURCES} ${IMGUI_SOURCES})


# link opengl libs
set (OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
target_link_libraries(syncwatchUI ${OPENGL_LIBRARIES})

if (WIN32)
# Imm32 is required for imgui
# mingw32 should be before SDL2main
    target_link_libraries(syncwatchUI Imm32 mingw32 glew32)
endif()

# link sdl2 libraries
find_package(SDL2 REQUIRED HINTS $ENV{SDL_DIR})
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(syncwatchUI SDL2::SDL2)
add_definitions(-DSDL_MAIN_HANDLED)

# link mpv libraries
target_link_libraries(syncwatchUI mpv)

# include youtube-dl.exe on windows
if (WIN32)
    install(FILES "youtube-dl.exe" DESTINATION .)
endif()
# TODO: on linux optional package dependency to youtube-dl

# link fmt (fmt::format is used for string formatting)
target_link_libraries(syncwatchUI fmt)

# link libtorrent
target_link_libraries(syncwatchUI torrent-rasterbar)
if (WIN32)
    target_link_libraries(syncwatchUI ws2_32)
endif()

# cpack
#install(TARGETS syncwatchUI RUNTIME_DEPENDENCIES DESTINATION .)
#include(InstallRequiredSystemLibraries)
#include(CPack)