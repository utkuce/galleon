// Build with: gcc -o main main.c `pkg-config --libs --cflags mpv sdl2` -std=c99

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <string>
#include <iostream>

#include "video_player.h"
#include "../torrent/torrent_handler.h"
#include "../interface/interface.h"

void die(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void *get_proc_address_mpv(void *fn_ctx, const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

void on_mpv_events(void *ctx)
{
    SDL_Event event = {.type = wakeup_on_mpv_events};
    SDL_PushEvent(&event);
}

void on_mpv_render_update(void *ctx)
{
    SDL_Event event = {.type = wakeup_on_mpv_render_update};
    SDL_PushEvent(&event);
}

Uint32 wakeup_on_mpv_render_update, wakeup_on_mpv_events;
mpv_handle *mpv;
mpv_render_context *mpv_gl;
int redraw;
int position, duration;

void initialize_mpv()
{
    mpv = mpv_create();
    if (!mpv)
        die("context init failed");


    mpv_set_option_string(mpv, "input-ipc-server", "\\\\.\\pipe\\syncwatch-socket");
    //mpv_set_option_string(mpv, "log-file", "mpv-log.txt");  

    // Some minor options can only be set before mpv_initialize().
    if (mpv_initialize(mpv) < 0)
        die("mpv init failed");

    mpv_request_log_messages(mpv, "debug");

    mpv_opengl_init_params opengl_params{
        .get_proc_address = get_proc_address_mpv
    };

    int advanced_control = 1;

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_OPENGL},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &opengl_params},
        // Tell libmpv that you will call mpv_render_context_update() on render
        // context update callbacks, and that you will _not_ block on the core
        // ever (see <libmpv/render.h> "Threading" section for what libmpv
        // functions you can call at all when this is active).
        // In particular, this means you must call e.g. mpv_command_async()
        // instead of mpv_command().
        // If you want to use synchronous calls, either make them on a separate
        // thread, or remove the option below (this will disable features like
        // DR and is not recommended anyway).
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advanced_control},
        {mpv_render_param{}}
    };

    // This makes mpv use the currently set GL context. It will use the callback
    // (passed via params) to resolve GL builtin functions, as well as extensions.
    
    if (mpv_render_context_create(&mpv_gl, mpv, params) < 0)
        die("failed to initialize mpv GL context");

    // We use events for thread-safe notification of the SDL main loop.
    // Generally, the wakeup callbacks (set further below) should do as least
    // work as possible, and merely wake up another thread to do actual work.
    // On SDL, waking up the mainloop is the ideal course of action. SDL's
    // SDL_PushEvent() is thread-safe, so we use that.
    wakeup_on_mpv_render_update = SDL_RegisterEvents(1);
    wakeup_on_mpv_events = SDL_RegisterEvents(1);
    if (wakeup_on_mpv_render_update == (Uint32)-1 ||
        wakeup_on_mpv_events == (Uint32)-1)
        die("could not register events");

    // When normal mpv events are available.
    mpv_set_wakeup_callback(mpv, on_mpv_events, NULL);

    // When there is a need to call mpv_render_context_update(), which can
    // request a new frame to be rendered.
    // (Separate from the normal event handling mechanism for the sake of
    //  users which run OpenGL on a different thread.)
    mpv_render_context_set_update_callback(mpv_gl, on_mpv_render_update, NULL);

    redraw = 0;
}

void mpv_events(SDL_Event event)
{
    //redraw = 0;

    // Happens when there is new work for the render thread (such as
    // rendering a new video frame or redrawing it).
    if (event.type == wakeup_on_mpv_render_update) {
        uint64_t flags = mpv_render_context_update(mpv_gl);
        if (flags & MPV_RENDER_UPDATE_FRAME)
            redraw = 1;
    }
    // Happens when at least 1 new event is in the mpv event queue.
    if (event.type == wakeup_on_mpv_events) {
        // Handle all remaining mpv events.
        while (1) {

            mpv_event *mp_event = mpv_wait_event(mpv, 0);

            if (mp_event->event_id == MPV_EVENT_NONE)
                break;

            video_event(mp_event);
        }
    }
}

void mpv_redraw(SDL_Window *window)
{
    if (redraw) {

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        mpv_opengl_fbo opengl_fbo{
            .fbo = 0,
            .w = w,
            .h = h,
        };

        int flip_y = 1;

        mpv_render_param params[] = {
            // Specify the default framebuffer (0) as target. This will
            // render onto the entire screen. If you want to show the video
            // in a smaller rectangle or apply fancy transformations, you'll
            // need to render into a separate FBO and draw it manually.
            {MPV_RENDER_PARAM_OPENGL_FBO, &opengl_fbo},
            // Flip rendering (needed due to flipped GL coordinate system).
            {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
            {mpv_render_param_type{}}
        };

        // See render_gl.h on what OpenGL environment mpv expects, and
        // other API details.
        mpv_render_context_render(mpv_gl, params);
        //SDL_GL_SwapWindow(window);
    }
}

void mpv_toggle_pause()
{
    const char *cmd_pause[] = {"cycle", "pause", NULL};
    mpv_command_async(mpv, 0, cmd_pause);
}

void mpv_seek(const char* value, const char* type)
{
    const char *seek[] = {"seek", value, type, NULL};
    mpv_command_async(mpv, 0, seek);
}

void set_video_source(const char* source)
{
    if (std::string(source).rfind("magnet:", 0) == 0) 
    {
        std::cout << "Received magnet link, forwarding to torrent handler" << std::endl;
        torrent::add_magnet_link(source);
    }
    else
    {
        const char *cmd[] = {"loadfile", source, NULL};
        mpv_command_async(mpv, 0, cmd);
    }

    interface::loading_source = true;
    videoevent::loaded_video = false;
}