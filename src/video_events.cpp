#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <fmt/core.h>

#include "video_player.h"
#include "interface.h"

int player_ready = 0;
bool videoevent::paused;
bool videoevent::loaded_video = false;
std::string default_video = "https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";
static int seeking = 0;
std::map<int, std::string> interface::subtitle_list;

void video_event(mpv_event *mp_event)
{
    if (mp_event->event_id == MPV_EVENT_PROPERTY_CHANGE)
    {
        mpv_event_property *prop = (mpv_event_property*)(mp_event->data);

        if (strcmp(prop->name, "duration") == 0)
        {
            duration = *(int *)(prop->data);
            std::cout << "property change: " << prop->name << 
                " value: " << duration << std::endl;
        }

        if (strcmp(prop->name, "playback-time") == 0)
        {
            position = *(int *)(prop->data);
            
            if (seeking)
                std::cout << "property change: " << prop->name << 
                    " value: " << position << std::endl;
        }

        if (strcmp(prop->name, "media-title") == 0)
        {
            char* media_title = *(char **)(prop->data);
            std::cout << "property change: " << prop->name << 
                " value: " << media_title << std::endl;

            interface::set_media_title(media_title);

            const char *cmd[] = {"set", "pause", "yes", NULL};
            mpv_command_async(mpv, 0, cmd);
        }

        if (strcmp(prop->name, "pause") == 0)
        {
            videoevent::paused = *(int *)(prop->data);
            std::cout << "property change: " << prop->name << 
                " value: " << videoevent::paused << std::endl;
        }

        if (strcmp(prop->name, "seeking") == 0)
        {
            seeking = *(int *)(prop->data);
            std::cout << "property change: " << prop->name << 
                " value: " << seeking << std::endl;
        }

        // https://mpv.io/manual/master/#command-interface-track-list
        // https://www.ccoderun.ca/programming/doxygen/mpv/structmpv__node.html
        if (strcmp(prop->name, "track-list") == 0)
        {
            mpv_node node = *(mpv_node *)(prop->data);
            mpv_node_list* tracks = node.u.list;
            int number_of_tracks = tracks->num;

            std::cout << "Number of tracks: " << number_of_tracks << std::endl; 

            for (int i = 0; i < number_of_tracks; i++)
            {
                auto track_properties = tracks->values[i].u.list;
                char* track_type = track_properties->values[1].u.string;
                std::cout << "Track " << i << ", type: " << track_type;

                if (strcmp(track_type, "sub") == 0)
                {
                    int sid = track_properties->values[0].u.int64;
                    char* subtitle_title = track_properties->values[3].u.string;

                    std::string subtitle_title_str;
                    if (track_properties->values[3].format != MPV_FORMAT_STRING)
                        subtitle_title_str = fmt::format("Subtitle {}", sid);
                    else
                        subtitle_title_str = std::string(subtitle_title);

                    std::cout << " -> sid: " << sid << " title: " <<  subtitle_title_str;
                    interface::subtitle_list[sid] = subtitle_title_str;
                }

                std::cout << std::endl;
            }
        }
    }

    const char *event_name = mpv_event_name(mp_event->event_id);
    const char *event_message = "";

    if (mp_event->event_id == MPV_EVENT_LOG_MESSAGE)
    {
        mpv_event_log_message* msg = (mpv_event_log_message*)(mp_event->data);
        event_message = msg->text;
        //printf("event: %s, data: %s", event_name, event_message);
        //if (strstr(msg->text, "DR image"))
        //    printf("log: %s", msg->text);
    }
    else
    {
        //printf("event: %s\n", event_name);
        //fflush(stdout);
    }

    if (mp_event->event_id == MPV_EVENT_FILE_LOADED)
    {
        videoevent::loaded_video = true;
    }
}
