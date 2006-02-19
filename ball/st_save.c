/*   
 * Copyright (C) 2003 Robert Kooima
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

#include <string.h>

#include "gui.h"
#include "game.h"
#include "util.h"
#include "demo.h"
#include "set.h"
#include "level.h"
#include "audio.h"
#include "config.h"

#include "st_save.h"
#include "st_over.h"
#include "st_done.h"
#include "st_level.h"
#include "st_title.h"

/*---------------------------------------------------------------------------*/

#define SAVE_SAVE   2
#define SAVE_CANCEL 3

static int  file_id;
static char filename[MAXNAM];

static int save_action(int i)
{
    size_t l = strlen(filename);
    int next = level_mode() == MODE_CHALLENGE;

    audio_play(AUD_MENU, 1.0f);

    switch (i)
    {
    case SAVE_SAVE:
	if (strcmp(filename, "") == 0)
	    return 1;
        if (demo_exists(filename))
            return goto_state(&st_clobber);
        else
        {
            if (level_exit(filename, next))
                return goto_state(&st_level);
	    else if (!level_dead() && level_mode() == MODE_CHALLENGE)
                return goto_state(&st_done);
	    else
		return goto_state(&st_title);
        }

    case SAVE_CANCEL:
        if (level_exit(NULL, next))
            return goto_state(&st_level);
	else if (!level_dead() && level_mode() == MODE_CHALLENGE)
            return goto_state(&st_done);
        else
	    return goto_state(&st_title);

    case GUI_CL:
        gui_keyboard_lock();
        break;

    case GUI_BS:
        if (l > 0)
        {
            filename[l - 1] = 0;
            gui_set_label(file_id, filename);
        }
        break;

    default:
        if (l < MAXNAM - 1)
        {
            filename[l + 0] = gui_keyboard_char((char) i);
            filename[l + 1] = 0;
            gui_set_label(file_id, filename);
        }
    }
    return 1;
}

static int save_enter(void)
{
    int id, jd, kd, ld;

    demo_unique(filename);

    if ((id = gui_vstack(0)))
    {
        gui_space(id);
        if ((jd = gui_hstack(id)))
        {
            gui_filler(jd);
	    if ((kd = gui_vstack(jd)))
	    {
		if ((ld = gui_hstack(kd)))
		{
		    gui_count(ld, curr_level(), GUI_LRG, GUI_NE);
		    gui_label(ld, _("Level "),  GUI_LRG, GUI_NW, 0, 0);
		}
		gui_label(kd, _(set_name(set_curr())),  GUI_SML, GUI_BOT, gui_wht, gui_wht);
	    }
	    gui_filler(jd);
        }
        gui_space(id);

        gui_space(id);
        file_id = gui_label(id, filename, GUI_MED, GUI_ALL, gui_yel, gui_yel);
        gui_space(id);

        if ((jd = gui_harray(id)))
        {
            gui_start(jd, _("Cancel"), GUI_SML, SAVE_CANCEL, 0);
            gui_start(jd, _("Save"),   GUI_SML, SAVE_SAVE,   0);
        }
        gui_keyboard(id);
        
        gui_layout(id, 0, 0);
    }

    return id;
}

static void save_leave(int id)
{
    gui_delete(id);
}

static void save_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void save_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static void save_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void save_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int save_click(int b, int d)
{
    if (b <= 0 && d == 1)
        return save_action(gui_token(gui_click()));
    return 1;
}

static int save_keybd(int c, int d)
{
    if (d && c == SDLK_ESCAPE)
        goto_state(&st_over);
    return 1;
}

static int save_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return save_click(0, 1);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return goto_state(&st_over);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int clobber_action(int i)
{
    int next = level_mode() == MODE_CHALLENGE;
    audio_play(AUD_MENU, 1.0f);

    if (i == SAVE_SAVE)
    {
            if (level_exit(filename, next))
                return goto_state(&st_level);
            else
                return goto_state(&st_title);
    }
    return goto_state(&st_save);
}

static int clobber_enter(void)
{
    int id, jd, kd;

    if ((id = gui_vstack(0)))
    {
        kd = gui_label(id, _("Overwrite?"), GUI_MED, GUI_ALL, gui_red, gui_red);

        gui_label(id, filename, GUI_MED, GUI_ALL, gui_yel, gui_yel);

        if ((jd = gui_harray(id)))
        {
            gui_state(jd, _("Yes"), GUI_SML, SAVE_SAVE,   0);
            gui_start(jd, _("No"),  GUI_SML, SAVE_CANCEL, 1);
        }

        gui_pulse(kd, 1.2f);
        gui_layout(id, 0, 0);
    }

    return id;
}

static void clobber_leave(int id)
{
    gui_delete(id);
}

static void clobber_paint(int id, float st)
{
    game_draw(0, st);
    gui_paint(id);
}

static void clobber_timer(int id, float dt)
{
    gui_timer(id, dt);
    audio_timer(dt);
}

static int clobber_keybd(int c, int d)
{
    return (d && c == SDLK_ESCAPE) ? clobber_action(SAVE_CANCEL) : 1;
}

static void clobber_point(int id, int x, int y, int dx, int dy)
{
    gui_pulse(gui_point(id, x, y), 1.2f);
}

static void clobber_stick(int id, int a, int v)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        gui_pulse(gui_stick(id, v, 0), 1.2f);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        gui_pulse(gui_stick(id, 0, v), 1.2f);
}

static int clobber_click(int b, int d)
{
    if (d && b < 0)
        return clobber_action(gui_token(gui_click()));
    return 1;
}

static int clobber_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return clobber_action(gui_token(gui_click()));
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return clobber_action(SAVE_CANCEL);
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_save = {
    save_enter,
    save_leave,
    save_paint,
    save_timer,
    save_point,
    save_stick,
    save_click,
    save_keybd,
    save_buttn,
    1, 0
};

struct state st_clobber = {
    clobber_enter,
    clobber_leave,
    clobber_paint,
    clobber_timer,
    clobber_point,
    clobber_stick,
    clobber_click,
    clobber_keybd,
    clobber_buttn,
    1, 0
};
