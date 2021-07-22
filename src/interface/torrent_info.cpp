#include <iostream>
#include "interface.h"
#include "../torrent/torrent_handler.h"

void interface::draw_torrent_info()
{
    // TODO: if there is no torrent session return

    ImGui::SetNextWindowSize(ImVec2(interface::panel_width, 
                             ImGui::GetTextLineHeightWithSpacing() * 3));
    
    ImGui::SetNextWindowPos(ImVec2(width - (margin + interface::panel_width), 
                                   ImGui::GetTextLineHeightWithSpacing() * 2 + margin*2));

    ImGui::Begin("Torrent Info", 0, ImGuiWindowFlags_NoCollapse | 
                                    ImGuiWindowFlags_AlwaysAutoResize | 
                                    ImGuiWindowFlags_NoTitleBar);

    ImGui::Text(torrent::progress != 1.0f ? "Downloading" : "Seeding");
    ImGui::SameLine();
    ImGui::TextDisabled("file (?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(torrent::name.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    ImGui::Text(std::string((torrent::progress != 1.0f ? "from " + std::to_string(torrent::peers) + " peers" : "")).c_str());

    ImGui::ProgressBar(torrent::progress, ImVec2(0.0f, 0.0f));
    ImGui::SameLine();

    ImGui::Text(torrent::progress != 1.0f ? torrent::speed.c_str() : "Done");

    ImGui::End();
}