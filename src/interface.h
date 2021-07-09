#ifndef INTERFACE_H
#define INTERFACE_H

#include "imgui/imgui.h"
#include <SDL2/SDL.h>

namespace interface 
{
    void init(SDL_Window*);
    void draw();
    void toggle_fullscreen();
    void set_fullscreen(bool);
    void set_last_mouse_motion(int);
    void set_media_title(char*);

    extern SDL_Window* sdl_window;
}

#endif

