/*
 * Copyright (C) 2003-2011 Neverball authors
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

#include <SDL.h>

#include "glext.h"

/*---------------------------------------------------------------------------*/

#ifndef CONF_OPENGLES

PFNGLCLIENTACTIVETEXTURE_PROC glClientActiveTexture_;
PFNGLACTIVETEXTURE_PROC       glActiveTexture_;

PFNGLGENBUFFERS_PROC          glGenBuffers_;
PFNGLBINDBUFFER_PROC          glBindBuffer_;
PFNGLBUFFERDATA_PROC          glBufferData_;
PFNGLBUFFERSUBDATA_PROC       glBufferSubData_;
PFNGLDELETEBUFFERS_PROC       glDeleteBuffers_;
PFNGLISBUFFER_PROC            glIsBuffer_;

PFNGLPOINTPARAMETERFV_PROC    glPointParameterfv_;

#endif

/*---------------------------------------------------------------------------*/

int glext_check(const char *needle)
{
    const GLubyte *haystack, *c;

    /* Search for the given string in the OpenGL extension strings. */

    for (haystack = glGetString(GL_EXTENSIONS); *haystack; haystack++)
    {
        for (c = (const GLubyte *) needle; *c && *haystack; c++, haystack++)
            if (*c != *haystack)
                break;

        if ((*c == 0) && (*haystack == ' ' || *haystack == '\0'))
            return 1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

#define SDL_GL_GFPA(fun, str) do {     \
    ptr = SDL_GL_GetProcAddress(str);   \
    memcpy(&fun, &ptr, sizeof (void *)); \
} while(0)

/*---------------------------------------------------------------------------*/

void glext_init(void)
{
#ifndef CONF_OPENGLES

    void *ptr;

    if (glext_check("ARB_multitexture"))
    {
        SDL_GL_GFPA(glClientActiveTexture_, "glClientActiveTextureARB");
        SDL_GL_GFPA(glActiveTexture_,       "glActiveTextureARB");
    }

    if (glext_check("ARB_vertex_buffer_object"))
    {
        SDL_GL_GFPA(glGenBuffers_,          "glGenBuffersARB");
        SDL_GL_GFPA(glBindBuffer_,          "glBindBufferARB");
        SDL_GL_GFPA(glBufferData_,          "glBufferDataARB");
        SDL_GL_GFPA(glBufferSubData_,       "glBufferSubDataARB");
        SDL_GL_GFPA(glDeleteBuffers_,       "glDeleteBuffersARB");
        SDL_GL_GFPA(glIsBuffer_,            "glIsBufferARB");
    }

    if (glext_check("ARB_point_parameters"))
        SDL_GL_GFPA(glPointParameterfv_,   "glPointParameterfvARB");

#endif
}

/*---------------------------------------------------------------------------*/

void glClipPlane4f(GLenum p, GLfloat a, GLfloat b, GLfloat c, GLfloat d)
{
#ifdef CONF_OPENGLES

    GLfloat v[4];

    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;

    glClipPlanef(p, v);

#else

    GLdouble v[4];

    v[0] = a;
    v[1] = b;
    v[2] = c;
    v[3] = d;

    glClipPlane(p, v);

#endif
}

/*---------------------------------------------------------------------------*/
