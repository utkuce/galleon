#include <iostream>

#include "interface.h"
#include "../video/video_player.h"

int slider_position;

using namespace interface;

const char* seconds_to_display(int, char*);

void interface::draw_media_controls()
{
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
    ImGui::Text("Show Info Panel");
    ImGui::SameLine();
    ImGui::Checkbox("##Show Panels", &show_input_panel);
    //ImGui::SameLine(0, 10);
    //ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

    ImGui::SameLine(0, 10);
    ImGui::Text("Volume");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    static int volume = 100.0f;
    if (ImGui::SliderInt("##Volume", &volume, 0, 100))
    {
        const char *cmd_volume[] = {"set", "volume", std::to_string(volume).c_str(), NULL};
        mpv_command_async(mpv, 0, cmd_volume);
    }


    ImGui::SameLine(0, 10);
    ImGui::SetNextItemWidth(150);
    static int subtitle_selection = 0;        
    
    if (ImGui::BeginCombo("##subs", "Subtitles"))
    {
        std::map<int, std::string>::iterator it;
        for (it = subtitle_list.begin(); it != subtitle_list.end(); it++)
        {
            const bool is_selected = (subtitle_selection == it->first);
            if (ImGui::Selectable(subtitle_list[it->first].c_str(), is_selected))
            {
                subtitle_selection = it->first;
                ImGui::SetItemDefaultFocus();

                std::cout << "Selected subtitle track " << subtitle_selection << std::endl;
                const char *cmd_sub[] = {"set", "sid", std::to_string(subtitle_selection).c_str(), NULL};
                mpv_command_async(mpv, 0, cmd_sub);

            }
        }

        ImGui::EndCombo();
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