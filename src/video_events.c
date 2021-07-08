#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "video_player.h"

#ifdef __cplusplus
extern "C" char * get_word(char*, int);
#endif

void video_event(mpv_event *mp_event)
{
    /*
    if (mp_event->event_id == MPV_EVENT_LOG_MESSAGE) {
        mpv_event_log_message *msg = mp_event->data;
        // Print log messages about DR allocations, just to
        // test whether it works. If there is more than 1 of
        // these, it works. (The log message can actually change
        // any time, so it's possible this logging stops working
        // in the future.)
        if (strstr(msg->text, "DR image"))
            printf("log: %s", msg->text);
        continue;
    }*/

    if (mp_event->event_id == MPV_EVENT_PROPERTY_CHANGE)
    {
        mpv_event_property *prop = mp_event->data;

        if (strcmp(prop->name, "duration") == 0)
        {
            duration = *(int *)(prop->data);
            //printf("property change: %s, %d\n", prop->name, duration);
        }

        if (strcmp(prop->name, "playback-time") == 0)
        {
            position = *(int *)(prop->data);
            //printf("property change: %s, %d\n", prop->name, position);
        }
    }

    if (mp_event->event_id == MPV_EVENT_CLIENT_MESSAGE)
    {

        mpv_event_client_message *message = mp_event->data;

        if (strcmp(message->args[0], "torrentInfo") == 0)
            set_torrent_info(message->args);

        if (strcmp(message->args[0], "roomLink") == 0)
            set_room_link(message->args[1]);

        if (strcmp(message->args[0], "newPeer") == 0)
            set_new_peer(message->args[1]);
    }

    const char *event_name = mpv_event_name(mp_event->event_id);
    const char *event_message = "";

    if (mp_event->event_id == MPV_EVENT_LOG_MESSAGE)
    {
        mpv_event_log_message *msg = mp_event->data;
        event_message = msg->text;
        //printf("event: %s, data: %s", event_name, event_message);

        // if the message contains "Subs"
        // expected format is:  Subs  --sid=4 --slang=eng 'English' (subrip)
        if (strstr(event_message, "Subs") != NULL)
        {
            printf("event: %s, data: %s", event_name, event_message);

            // get the language i.e English
            //char* msg_copy1 = malloc(sizeof(event_message));
            //strcpy(msg_copy1, event_message);
            //const char* language = get_word(msg_copy1, 3);
            //printf("%s", msg_copy1);
            //free(msg_copy1);

            // get the sid i.e. --sid=4
            //char* msg_copy2 = malloc(sizeof(event_message));
            //strcpy(msg_copy2, event_message);
            //const char* sid = get_word(msg_copy2, 1);
            //free(msg_copy2);

            //printf("Language: %s, sid: %s\n", language, sid);
        }
    }
    else
        printf("event: %s\n", event_name);

    if (mp_event->event_id == MPV_EVENT_FILE_LOADED)
    {
        loaded_video = 1;
    }

    if (mp_event->event_id == MPV_EVENT_PAUSE)
    {
    }

    if (mp_event->event_id == MPV_EVENT_UNPAUSE)
    {
    }

    if (mp_event->event_id == MPV_EVENT_SEEK)
    {

    }
}

#ifdef __cplusplus
extern "C" {
#endif

    char* get_word(char* str, int word_index)
    {
        int i = 0;
        // Extract the first token
        char * token = strtok(str, " ");
        // loop through the string to extract all other tokens
        while( token != NULL && i < word_index) {
            token = strtok(NULL, " ");
            i++;
        }

        return token;
    }

#ifdef __cplusplus
}
#endif
