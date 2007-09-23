#ifndef LEVEL_H
#define LEVEL_H

#include "base_config.h"
#include "score.h"

/*---------------------------------------------------------------------------*/

struct level
{
    char file[PATHMAX];
    char back[PATHMAX];
    char grad[PATHMAX];
    char shot[PATHMAX];
    char song[PATHMAX];

    char message[MAXSTR];

    char version[MAXSTR];
    char author[MAXSTR];

    int time; /* Time limit   */
    int goal; /* Coins needed */

    struct
    {
        struct score best_times;
        struct score unlock_goal;
        struct score most_coins;
    }
    score;

    /* Set information. */

    struct set *set;

    int  number;

    /* String representation of the number (eg. "IV") */
    char repr[MAXSTR];

    int is_locked;
    int is_bonus;
    int is_completed;
};

int level_load(const char *, struct level *);

void level_dump_info(const struct level *);

/*---------------------------------------------------------------------------*/

/* A level for the playing */

struct level_game
{
    int mode;          /* game mode */
    const struct level *level; /* the level played */

    int goal;          /* coins needed */
    int time;          /* time limit */

    /* MODE_CHALLENGE only */
    int score;         /* coin total */
    int balls;         /* live count */
    int times;         /* time total */

    /* Once a level is finished */
    int state;         /* state ending */
    int coins;         /* coins collected */
    int timer;         /* time elapsed */

    /* rank = 3  => unclassed */
    int coin_rank;     /* rank in the level high-scores */
    int goal_rank;     /* rank in the level high-scores */
    int time_rank;     /* rank in the level high-scores */
    int score_rank;    /* rank in the set high-scores */
    int times_rank;    /* rank in the set high-scores */

    /* What about the game and the set? */
    int dead;          /* Is the game over and lost? */
    int win;           /* Is the game over and win? */
    int unlock;        /* Is the next level newly unlocked */
    const struct level *next_level; /* next level (NULL no next level) */
};

/*---------------------------------------------------------------------------*/

#define GAME_NONE 0     /* No event (or aborted) */
#define GAME_TIME 1     /* Time's up */
#define GAME_GOAL 2     /* Goal reached */
#define GAME_FALL 3     /* Fall out */

const char *state_to_str(int);

#endif
