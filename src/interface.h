#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>

#include "imgui/imgui.h"
#include <SDL2/SDL.h>

void init_interface();
void construct_interface(SDL_Window*);
void toggle_fullscreen(SDL_Window*);
void set_fullscreen(bool);
void set_last_mouse_motion(int);

#endif