#ifndef DEMO_H
#define DEMO_H

#include <time.h>
#include "level.h"

/*---------------------------------------------------------------------------*/

/* Demo information structure (header) */
struct demo
{
    char   name[MAXNAM];      /* demo basename */
    char   filename[PATHMAX]; /* demo path */

    /* The following reflects the file structure */
    /* magic number */
    /* replay file version */
    int    timer;           /* elapsed time */
    int    coins;           /* coin number */
    int    state;           /* how the replay end */
    int    mode;            /* game mode */
    time_t date;            /* date of creation */
    char   player[MAXNAM];  /* player name */
    char   shot[PATHMAX];   /* image filename */
    char   file[PATHMAX];   /* level filename */
    char   back[PATHMAX];   /* level bg filename */
    char   grad[PATHMAX];   /* level gradiant filename */
    char   song[PATHMAX];   /* level song filename */
    int    time;            /* time limit (! training mode) */
    int    goal;            /* coin to open the goal (! training mode) */
    int    score;           /* sum of coins (challenge mode) */
    int    balls;           /* number of balls (challenge mode) */
    int    times;           /* total time (challenge mode) */
    char   nb_version[20]; /* neverball version used */
};


/*---------------------------------------------------------------------------*/

#define MAXDEMO 256

int               demo_scan(void);
const char        *demo_pick(void);
const struct demo *get_demo(int);

const char * date_to_str(time_t);

int  demo_exists(char *);
void demo_unique(char *);

/*---------------------------------------------------------------------------*/

int  demo_play_init(const char *, const struct level *, 
		    const struct level_game *);
/*                    int, int, int, int, int, int);*/
void demo_play_step(float);
void demo_play_stop(const struct level_game *);
int  demo_play_saved(void);
void demo_play_save(const char *);

/*---------------------------------------------------------------------------*/

int  demo_replay_init(const char *, struct level_game *);
int  demo_replay_step(float *);
void demo_replay_stop(int);
void demo_replay_dump_info(void);
const struct demo *curr_demo_replay(void);
		
/*---------------------------------------------------------------------------*/

#endif
