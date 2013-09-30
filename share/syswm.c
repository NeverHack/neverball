/*
 * Copyright (C) 2003-2010 Neverball authors
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>

#include <SDL_video.h>
#include <SDL_syswm.h>

#include "syswm.h"
#include "base_config.h"
#include "image.h"
#include "lang.h"

/*---------------------------------------------------------------------------*/

void set_SDL_icon(SDL_Window *window, const char *filename)
{
#if !defined(__APPLE__) && !defined(_WIN32)
    SDL_Surface *icon;

    if ((icon = load_surface(filename)))
    {
        SDL_SetWindowIcon(window, icon);
        free(icon->pixels);
        SDL_FreeSurface(icon);
    }
#endif
    return;
}

/*---------------------------------------------------------------------------*/

