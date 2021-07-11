#include "interface.h"
#include <iostream>
#include <string>
#include <vector>

#include "video_player.h"
//#include "util.h"

SDL_Window* interface::sdl_window;
int margin = 40;
bool show_info_panel = true;

int slider_position;
int last_mouse_motion;
bool show_interface;

bool loading_source = false;
bool fullscreen = false;

void interface::init(SDL_Window* window)
{
    sdl_window = window;
    subtitle_list.push_back((char *)"None");

    //const char *cmd_keep[] = {"set", "keep-open", "yes", NULL};
    //mpv_command_async(mpv, 0, cmd_keep); // TODO: Fix segfault because of this
}

void interface::toggle_fullscreen()
{
    SDL_SetWindowFullscreen(sdl_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    fullscreen = !fullscreen;
}

void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

const char* seconds_to_display(int input, char* output)
{
    input = input % (24 * 3600); 
    int hours = input / 3600; 
  
    input %= 3600; 
    int minutes = input / 60 ; 
  
    input %= 60; 
    int seconds = input; 

    if (hours != 0)
        sprintf(output, "%02d:%02d:%02d", hours, minutes, seconds);
    else
        sprintf(output, "%02d:%02d", minutes, seconds);
    return output;
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
        int width, height;
        SDL_GetWindowSize(sdl_window, &width, &height);

        if (show_info_panel)
        {
            int next_window_width = 350;
            ImGui::SetNextWindowPos(ImVec2(width - (margin + next_window_width), margin));
            ImGui::SetNextWindowSize(ImVec2(next_window_width, ImGui::GetTextLineHeightWithSpacing() * 3));
            ImGui::Begin("Video", 0, ImGuiWindowFlags_NoCollapse | 
                                     ImGuiWindowFlags_AlwaysAutoResize | 
                                     ImGuiWindowFlags_NoTitleBar);


            //ImGui::Text("Video source");

            auto video_input = [](char* source) 
            { 
                std::cout << "source input:" << source << std::endl;

                const char *cmd[] = {"loadfile", source, NULL};
                mpv_command_async(mpv, 0, cmd);

                loading_source = true;
            };

            static char str1[1024] = "";
            if (ImGui::InputTextWithHint("##vidsrc", "Video Source", str1, 
                IM_ARRAYSIZE(str1), 
                ImGuiInputTextFlags_EnterReturnsTrue))
            {
                video_input(str1);
            }

            ImGui::SameLine();
            if (ImGui::Button("Stream"))
            {
                video_input(str1);
            }

            if (loading_source == true) 
            {
                ImGui::OpenPopup("loading video");
                loading_source = false;
                videoevent::loaded_video = false;
            }

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | 
                                    ImGuiWindowFlags_AlwaysAutoResize | 
                                    ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoMove;

            if (ImGui::BeginPopupModal("loading video", nullptr, flags)){
                
                ImGui::Text("Loading video...");
                
                if (videoevent::loaded_video)
                {
                    videoevent::loaded_video = false;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
            
            ImGui::SameLine();
            std::string help_marker = ""; //"Enter a magnet link or a video url (youtube etc.)\n";
            help_marker += "A complete list of supported sources can be found on";
            help_marker += "\nhttps://ytdl-org.github.io/youtube-dl/supportedsites.html";
            HelpMarker(help_marker.c_str());
            
            static char str2[1024] = "";
            if (ImGui::InputTextWithHint("##mpvcmd", "mpv command", str2, IM_ARRAYSIZE(str2), 
                ImGuiInputTextFlags_EnterReturnsTrue))
            {
                mpv_command_string(mpv, str2);
            }
            ImGui::SameLine();
            if (ImGui::Button("Run"))
            {
                mpv_command_string(mpv, str2);
            }    

            
            ImGui::SameLine();
            std::string help_marker_mpv = "https://mpv.io/manual/master/#list-of-input-commands\n";
            help_marker_mpv += "Use at your own risk, not every command might work as you expect\n";
            help_marker_mpv += "Example:\nset sid 4\nwill select the 4th subtitle track";
            HelpMarker(help_marker_mpv.c_str());
            

            ImGui::End();
        }

        // media controls
        ImGui::SetNextWindowSize(ImVec2(width - margin * 2, 0));
        ImGui::SetNextWindowPos(ImVec2(margin, height - ImGui::GetTextLineHeightWithSpacing() * 2 - margin));

        ImGui::Begin("Media Controls", 0, ImGuiWindowFlags_NoTitleBar);

        char slider_display[99], position_display[99], duration_display[99];
        seconds_to_display(position, position_display);
        seconds_to_display(duration, duration_display);
        sprintf(slider_display, "%s/%s", position_display, duration_display);

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderInt("", &slider_position, 0, duration, slider_display))
        {
            //std::cout << "Slider position: " << slider_position << std::endl;
            const char *cmd_seek[] = {"seek", std::to_string(slider_position).c_str(), "absolute", NULL};
            mpv_command_async(mpv, 0, cmd_seek);
        }
        else
        {
            if (ImGui::IsItemDeactivatedAfterEdit)
                slider_position = position;
        }

        if (ImGui::Button("Play/Pause"))
        {
            mpv_toggle_pause();
        }

        ImGui::SameLine(0, 10);
        if (ImGui::Button("Fullscreen"))
        {
            toggle_fullscreen();
        }

        ImGui::SameLine(0, 10);
        ImGui::Checkbox("Show Info Panel", &show_info_panel);
        //ImGui::SameLine(0, 10);
        //ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

        ImGui::SameLine(0, 10);
        ImGui::SetNextItemWidth(100);
        static int volume = 100.0f;
        if (ImGui::SliderInt("Volume", &volume, 0, 100))
        {
            const char *cmd_volume[] = {"set", "volume", std::to_string(volume).c_str(), NULL};
            mpv_command_async(mpv, 0, cmd_volume);
        }


        ImGui::SameLine(0, 10);
        ImGui::SetNextItemWidth(150);
        static int subtitle_selection = 0;        
        if (ImGui::Combo("Subtitles", &subtitle_selection, &subtitle_list[0], subtitle_list.size()))
        {
            const char *cmd_sub[] = {"set", "sid", std::to_string(subtitle_selection).c_str(), NULL};
            mpv_command_async(mpv, 0, cmd_sub);
        }

        /*
        ImGui::SameLine(0, 10);
        static bool mute;
        if (ImGui::Checkbox("Mute", &mute))
        {
            const char *cmd_mute[] = {"set", "mute", mute ? "yes" : "no", NULL};
            mpv_command_async(mpv, 0, cmd_mute);
        }
        */

        ImGui::End();
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