#include <iostream>
#include "interface.h"
#include "../torrent/torrent_handler.h"

using namespace interface;

void HelpMarker(const char*);

bool interface::loading_source = false;

void interface::draw_input_panel()
{
    ImGui::SetNextWindowPos(ImVec2(width - (margin + interface::panel_width), margin));
    ImGui::SetNextWindowSize(ImVec2(interface::panel_width, ImGui::GetTextLineHeightWithSpacing() * 3));
    ImGui::Begin("Video", 0, ImGuiWindowFlags_NoCollapse | 
                             ImGuiWindowFlags_AlwaysAutoResize | 
                             ImGuiWindowFlags_NoTitleBar);


    //ImGui::Text("Video source");

    auto video_input = [](char* source) 
    { 
        std::cout << "source input:" << source << std::endl;
        set_video_source(source);
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