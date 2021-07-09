#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <iostream>

#include "video_player.h"
#include "interface.h"

int player_ready = 0;
bool videoevent::paused;
bool videoevent::loaded_video = false;
std::string default_video = "https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";

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
            //std::cout << "property change: " << prop->name << 
            //    " value: " << position << std::endl;
        }

        if (strcmp(prop->name, "media-title") == 0)
        {
            char* media_title = *(char **)(prop->data);
            std::cout << "property change: " << prop->name << 
                " value: " << media_title << std::endl;

            interface::set_media_title(media_title);
        }

        if (strcmp(prop->name, "pause") == 0)
        {
            videoevent::paused = *(int *)(prop->data);
            std::cout << "property change: " << prop->name << 
                " value: " << videoevent::paused << std::endl;
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
