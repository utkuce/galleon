#include "interface.h"
#include <iostream>
#include <string>
#include <vector>

SDL_Window* interface::sdl_window;

int interface::width, interface::height;
int interface::margin = 40;

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

std::string torrent_name;
float torrent_progress = 0.0f;
std::string torrent_speed = "0 mb/s";
std::string torrent_time = "unknown";
std::string torrent_peers = "<n>";

std::vector<std::string> peers;
std::string room_link = "not_set";

void set_room_link(const char* rl)
{
    room_link = rl;
}

void set_torrent_info(const char** torrent_info)
{
    torrent_name = torrent_info[1];
    torrent_progress = atof(torrent_info[2]);
    torrent_speed = torrent_info[3];
    torrent_time = torrent_info[4];
    torrent_peers = torrent_info[5];
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

void set_new_peer(const char* peer_id)
{
    peers.push_back(std::string(peer_id));
}