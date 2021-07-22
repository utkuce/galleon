#include "interface.h"
#include <iostream>
#include <string>
#include <vector>

SDL_Window* interface::sdl_window;

int interface::width, interface::height;
int interface::margin = 40;
int interface::panel_width = 350;

bool interface::show_input_panel = true;

int last_mouse_motion;
bool show_interface;

bool fullscreen = false;

void interface::init(SDL_Window* window)
{
    sdl_window = window;
    subtitle_list[0] = "None";

    //const char *cmd_keep[] = {"set", "keep-open", "yes", NULL};
    //mpv_command_async(mpv, 0, cmd_keep); // TODO: Fix segfault because of this
}

void interface::toggle_fullscreen()
{
    SDL_SetWindowFullscreen(sdl_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    fullscreen = !fullscreen;
}

void interface::draw()
{
    show_interface = ImGui::GetIO().WantCaptureMouse || 
                     ImGui::GetIO().WantCaptureKeyboard ||
                     SDL_GetTicks() - last_mouse_motion < 2000 ||
                     videoevent::paused;

    //ImGui::ShowDemoWindow();
    SDL_ShowCursor(show_interface);

    if (show_interface)
    {
        SDL_GetWindowSize(sdl_window, &width, &height);

        if (show_input_panel)
        {
            draw_input_panel();
            draw_torrent_info();
        }

        draw_media_controls();
    }
}

void interface::set_media_title(char* title)
{
    SDL_SetWindowTitle(sdl_window, title);
}

void interface::set_fullscreen(bool value)
{
    fullscreen = value;
}

void interface::set_last_mouse_motion(int timestamp)
{
    last_mouse_motion = timestamp;
}