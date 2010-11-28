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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "glext.h"
#include "config.h"
#include "video.h"
#include "image.h"
#include "set.h"
#include "common.h"
#include "fs.h"

#include "game_server.h"
#include "game_client.h"
#include "game_proxy.h"

/*---------------------------------------------------------------------------*/

struct set
{
    char file[PATHMAX];

    char *id;                  /* Internal set identifier    */
    char *name;                /* Set name                   */
    char *desc;                /* Set description            */
    char *shot;                /* Set screen-shot            */

    char *user_scores;         /* User high-score file       */
    char *cheat_scores;        /* Cheat mode score file      */

    struct score coin_score;   /* Challenge score            */
    struct score time_score;   /* Challenge score            */

    /* Level info */

    int   count;                /* Number of levels           */
    char *level_name_v[MAXLVL]; /* List of level file names   */
};

#define SET_GET(a, i) ((struct set *) array_get((a), (i)))

static Array sets;
static int   curr;

static struct level level_v[MAXLVL];

/*---------------------------------------------------------------------------*/

static void put_score(fs_file fp, const struct score *s)
{
    int i;

    for (i = RANK_HARD; i <= RANK_EASY; i++)
        fs_printf(fp, "%d %d %s\n", s->timer[i], s->coins[i], s->player[i]);
}

void set_store_hs(void)
{
    const struct set *s = SET_GET(sets, curr);
    fs_file fout;
    int i;
    const struct level *l;
    char states[MAXLVL + 1];

    if ((fout = fs_open(config_cheat() ?
                        s->cheat_scores :
                        s->user_scores, "w")))
    {
        for (i = 0; i < s->count; i++)
        {
            if (level_v[i].is_locked)
                states[i] = 'L';
            else if (level_v[i].is_completed)
                states[i] = 'C';
            else
                states[i] = 'O';
        }
        states[s->count] = '\0';
        fs_printf(fout, "%s\n",states);

        put_score(fout, &s->time_score);
        put_score(fout, &s->coin_score);

        for (i = 0; i < s->count; i++)
        {
            l = &level_v[i];

            put_score(fout, &l->scores[SCORE_TIME]);
            put_score(fout, &l->scores[SCORE_GOAL]);
            put_score(fout, &l->scores[SCORE_COIN]);
        }

        fs_close(fout);
    }
}

static int get_score(fs_file fp, struct score *s)
{
    char line[MAXSTR];
    int i;

    for (i = RANK_HARD; i <= RANK_EASY; i++)
    {
        int n = -1;

        if (!fs_gets(line, sizeof (line), fp))
            return 0;

        strip_newline(line);

        if (sscanf(line, "%d %d %n", &s->timer[i], &s->coins[i], &n) < 2)
            return 0;

        if (n < 0)
            return 0;

        SAFECPY(s->player[i], line + n);
    }

    return 1;
}

static void set_load_hs(void)
{
    struct set *s = SET_GET(sets, curr);
    fs_file fin;
    int i;
    int res = 0;
    struct level *l;
    const char *fn = config_cheat() ? s->cheat_scores : s->user_scores;
    char states[MAXLVL + sizeof ("\n")];

    if ((fin = fs_open(fn, "r")))
    {
        res = (fs_gets(states, sizeof (states), fin) &&
               strlen(states) - 1 == s->count);

        for (i = 0; i < s->count && res; i++)
        {
            switch (states[i])
            {
            case 'L':
                level_v[i].is_locked = 1;
                level_v[i].is_completed = 0;
                break;

            case 'C':
                level_v[i].is_locked = 0;
                level_v[i].is_completed = 1;
                break;

            case 'O':
                level_v[i].is_locked = 0;
                level_v[i].is_completed = 0;
                break;

            default:
                res = 0;
            }
        }

        res = res &&
            get_score(fin, &s->time_score) &&
            get_score(fin, &s->coin_score);

        for (i = 0; i < s->count && res; i++)
        {
            l = &level_v[i];
            res = (get_score(fin, &l->scores[SCORE_TIME])  &&
                   get_score(fin, &l->scores[SCORE_GOAL]) &&
                   get_score(fin, &l->scores[SCORE_COIN]));
        }

        fs_close(fin);

        if (!res)
            fprintf(stderr, L_("Failure to load user score file '%s'\n"), fn);
    }
}

/*---------------------------------------------------------------------------*/

static int set_load(struct set *s, const char *filename)
{
    fs_file fin;
    char *scores, *level_name;

    /* Skip "Misc" set when not in dev mode. */

    if (strcmp(filename, SET_MISC) == 0 && !config_cheat())
        return 0;

    fin = fs_open(filename, "r");

    if (!fin)
    {
        fprintf(stderr, L_("Failure to load set file '%s'\n"), filename);
        return 0;
    }

    memset(s, 0, sizeof (struct set));

    /* Set some sane values in case the scores are missing. */

    score_init_hs(&s->time_score, 359999, 0);
    score_init_hs(&s->coin_score, 359999, 0);

    SAFECPY(s->file, filename);

    if (read_line(&s->name, fin) &&
        read_line(&s->desc, fin) &&
        read_line(&s->id,   fin) &&
        read_line(&s->shot, fin) &&
        read_line(&scores,  fin))
    {
        sscanf(scores, "%d %d %d %d %d %d",
               &s->time_score.timer[RANK_HARD],
               &s->time_score.timer[RANK_MEDM],
               &s->time_score.timer[RANK_EASY],
               &s->coin_score.coins[RANK_HARD],
               &s->coin_score.coins[RANK_MEDM],
               &s->coin_score.coins[RANK_EASY]);

        free(scores);

        s->user_scores  = concat_string("Scores/", s->id, ".txt",       NULL);
        s->cheat_scores = concat_string("Scores/", s->id, "-cheat.txt", NULL);

        s->count = 0;

        while (s->count < MAXLVL && read_line(&level_name, fin))
        {
            s->level_name_v[s->count] = level_name;
            s->count++;
        }

        fs_close(fin);

        return 1;
    }

    free(s->name);
    free(s->desc);
    free(s->id);
    free(s->shot);

    fs_close(fin);

    return 0;
}

static void set_free(struct set *s)
{
    int i;

    free(s->name);
    free(s->desc);
    free(s->id);
    free(s->shot);

    free(s->user_scores);
    free(s->cheat_scores);

    for (i = 0; i < s->count; i++)
        free(s->level_name_v[i]);
}

/*---------------------------------------------------------------------------*/

static int cmp_dir_items(const void *A, const void *B)
{
    const struct dir_item *a = A, *b = B;
    return strcmp(a->path, b->path);
}

static int set_is_loaded(const char *path)
{
    int i;

    for (i = 0; i < array_len(sets); i++)
        if (strcmp(SET_GET(sets, i)->file, path) == 0)
            return 1;

    return 0;
}

static int is_unseen_set(struct dir_item *item)
{
    return (str_starts_with(base_name(item->path), "set-") &&
            str_ends_with(item->path, ".txt") &&
            !set_is_loaded(item->path));
}

int set_init()
{
    fs_file fin;
    char *name;

    Array items;
    int i;

    if (sets)
        set_quit();

    sets = array_new(sizeof (struct set));
    curr = 0;

    /*
     * First, load the sets listed in the set file, preserving order.
     */

    if ((fin = fs_open(SET_FILE, "r")))
    {
        while (read_line(&name, fin))
        {
            struct set *s = array_add(sets);

            if (!set_load(s, name))
                array_del(sets);

            free(name);
        }
        fs_close(fin);
    }

    /*
     * Then, scan for any remaining set description files, and add
     * them after the first group in alphabetic order.
     */

    if ((items = fs_dir_scan("", is_unseen_set)))
    {
        array_sort(items, cmp_dir_items);

        for (i = 0; i < array_len(items); i++)
        {
            struct set *s = array_add(sets);

            if (!set_load(s, DIR_ITEM_GET(items, i)->path))
                array_del(sets);
        }

        fs_dir_free(items);
    }

    return array_len(sets);
}

void set_quit(void)
{
    int i;

    for (i = 0; i < array_len(sets); i++)
        set_free(array_get(sets, i));

    array_free(sets);
    sets = NULL;
}

/*---------------------------------------------------------------------------*/

int set_exists(int i)
{
    return (0 <= i && i < array_len(sets));
}

const char *set_id(int i)
{
    return set_exists(i) ? SET_GET(sets, i)->id : NULL;
}

const char *set_name(int i)
{
    return set_exists(i) ? _(SET_GET(sets, i)->name) : NULL;
}

const char *set_desc(int i)
{
    return set_exists(i) ? _(SET_GET(sets, i)->desc) : NULL;
}

const char *set_shot(int i)
{
    return set_exists(i) ? SET_GET(sets, i)->shot : NULL;
}

const struct score *set_score(int i, int s)
{
    if (set_exists(i))
    {
        if (s == SCORE_TIME) return &SET_GET(sets, i)->time_score;
        if (s == SCORE_COIN) return &SET_GET(sets, i)->coin_score;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

static void set_load_levels(void)
{
    struct level *l;
    int nb = 1, bnb = 1;

    int i;

    const char *roman[] = {
        "",
        "I",   "II",   "III",   "IV",   "V",
        "VI",  "VII",  "VIII",  "IX",   "X",
        "XI",  "XII",  "XIII",  "XIV",  "XV",
        "XVI", "XVII", "XVIII", "XIX",  "XX",
        "XXI", "XXII", "XXIII", "XXIV", "XXV"
    };

    for (i = 0; i < SET_GET(sets, curr)->count; i++)
    {
        l = &level_v[i];

        level_load(SET_GET(sets, curr)->level_name_v[i], l);

        l->number = i;

        if (l->is_bonus)
            sprintf(l->name, "%s",   roman[bnb++]);
        else
            sprintf(l->name, "%02d", nb++);

        l->is_locked    = 1;
        l->is_completed = 0;

        if (i > 0) level_v[i - 1].next = l;
    }

    /* Unlock first level. */

    level_v[0].is_locked = 0;
}

void set_goto(int i)
{
    curr = i;

    set_load_levels();
    set_load_hs();
}

int curr_set(void)
{
    return curr;
}

struct level *get_level(int i)
{
    return (i >= 0 && i < SET_GET(sets, curr)->count) ? &level_v[i] : NULL;
}

/*---------------------------------------------------------------------------*/

int set_score_update(int timer, int coins, int *score_rank, int *times_rank)
{
    struct set *s = SET_GET(sets, curr);
    const char *player = config_get_s(CONFIG_PLAYER);

    score_coin_insert(&s->coin_score, score_rank, player, timer, coins);
    score_time_insert(&s->time_score, times_rank, player, timer, coins);

    if ((score_rank && *score_rank < RANK_LAST) ||
        (times_rank && *times_rank < RANK_LAST))
        return 1;
    else
        return 0;
}

void set_rename_player(int score_rank, int times_rank, const char *player)
{
    struct set *s = SET_GET(sets, curr);

    SAFECPY(s->coin_score.player[score_rank], player);
    SAFECPY(s->time_score.player[times_rank], player);
}

/*---------------------------------------------------------------------------*/

void level_snap(int i, const char *path)
{
    char *filename;

    /* Convert the level name to a PNG filename. */

    filename = concat_string(path,
                             "/",
                             base_name_sans(level_v[i].file, ".sol"),
                             ".png",
                             NULL);

    /* Initialize the game for a snapshot. */

    if (game_client_init(level_v[i].file))
    {
        union cmd cmd;
        cmd.type = CMD_GOAL_OPEN;
        game_proxy_enq(&cmd);
        game_client_sync(NULL);

        /* Render the level and grab the screen. */

        video_clear();
        game_client_fly(1.0f);
        game_kill_fade();
        game_client_draw(POSE_LEVEL, 0);
        SDL_GL_SwapBuffers();

        image_snap(filename);
    }

    free(filename);
}

void set_cheat(void)
{
    int i;

    for (i = 0; i < SET_GET(sets, curr)->count; i++)
    {
        level_v[i].is_locked    = 0;
        level_v[i].is_completed = 1;
    }
}

/*---------------------------------------------------------------------------*/
