#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mpv/client.h>
#include <mpv/render_gl.h>

extern Uint32 wakeup_on_mpv_render_update, wakeup_on_mpv_events;

void die(const char *msg);

void initialize_mpv();

extern mpv_handle *mpv;
extern mpv_render_context *mpv_gl;

extern int redraw;
void mpv_events(SDL_Event event);
void mpv_redraw(SDL_Window *window);

void mpv_input();

void mpv_toggle_pause();
void mpv_seek(const char*, const char*);
extern int position, duration;

void set_room_link(const char*);
void set_torrent_info(const char**);
void set_new_peer(const char*);

void video_event(mpv_event *);

// events
extern int loaded_video;

#ifdef __cplusplus
}
#endif

#endif