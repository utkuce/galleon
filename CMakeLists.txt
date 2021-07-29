cmake_minimum_required (VERSION 3.2)
project (syncwatchUI)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE Debug)

# source files for the main executable
file(GLOB MY_SOURCES "src/*.cpp" "src/*/*.cpp")
file(GLOB IMGUI_SOURCES "src/imgui/*.cpp")
add_executable(${PROJECT_NAME} ${MY_SOURCES} ${IMGUI_SOURCES})


# link opengl libs
set (OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

if (WIN32)
# Imm32 is required for imgui
# mingw32 should be before SDL2main
    target_link_libraries(${PROJECT_NAME} Imm32 mingw32 glew32)
endif()

# link sdl2 libraries
find_package(SDL2 REQUIRED HINTS $ENV{SDL_DIR})
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
add_definitions(-DSDL_MAIN_HANDLED)

# link mpv libraries
target_link_libraries(${PROJECT_NAME} mpv)

# link fmt (fmt::format is used for string formatting)
target_link_libraries(${PROJECT_NAME} fmt)

# link libtorrent
target_link_libraries(${PROJECT_NAME} torrent-rasterbar)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

if (WIN32)
    # include youtube-dl.exe on windows
    install(FILES "youtube-dl.exe" DESTINATION .)
    #install(FILES "C:/msys64/mingw64/bin/mpv-1.dll" DESTINATION .)

    install(TARGETS ${PROJECT_NAME} DESTINATION .
    RUNTIME_DEPENDENCIES
        DIRECTORIES "C:/msys64/mingw64/bin/"
        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
    )

endif()
# TODO: on linux optional package dependency to youtube-dl

set(CPACK_PACKAGING_INSTALL_PREFIX "/bin")

set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_NAME})
set(CPACK_NSIS_MODIFY_PATH "ON")
set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})

include(InstallRequiredSystemLibraries)
include(CPack)