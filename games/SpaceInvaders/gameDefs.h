#ifndef __GAME_DEFS
#define __GAME_DEFS

// If set, player cannot be killed by alien bombs
//#define PLAYER_IMMUNE

// If set, audio will be played in demo/attract mode
//#define ENABLE_AUDIO_IN_DEMO_MODE

#define INVADERS_PER_ROW    11
#define INVADER_ROWS        5
#define NUM_INVADERS        (INVADERS_PER_ROW*INVADER_ROWS)
#define NUM_MISSILES        4

// Used in bit tests
#define NO_INVADER      -1
#define INVADER_HERE    0
#define INVADER_LEFT    1
#define INVADER_RIGHT   2
#define INVADER_BOTH    3

// Used to handle various game events
#define INVADERS_DROPPING_BIT       0
#define INVADERS_PAUSED_BIT         1
#define INVADER_EXPLODING_BIT       2
#define SAUCER_ACTIVE_BIT           3
#define SAUCER_EXPLODING_BIT        4
#define PLAYER_EXPLODING_BIT        5
#define PLAYER_DEAD_BIT             6
#define DEMO_MODE_BIT               7
#define SHOW_PLAYER_BIT             8
#define FIRE_LOCKOUT_BIT            9
#define OPTION_LOCKOUT_BIT          10
#define TWO_PLAYER_GAME_BIT         11
#define ALIENS_LANDED_BIT           12
#define NEW_HI_BIT                  13


#define INVADERS_DROPPING           (gameData.gameFlags&(1<<INVADERS_DROPPING_BIT))
#define INVADERS_PAUSED             (gameData.gameFlags&(1<<INVADERS_PAUSED_BIT))
#define INVADERS_EXPLODING          (gameData.gameFlags&(1<<INVADERS_EXPLODING_BIT))
#define SAUCER_ACTIVE               (gameData.gameFlags&(1<<SAUCER_ACTIVE_BIT))
#define SAUCER_EXPLODING            (gameData.gameFlags&(1<<SAUCER_EXPLODING_BIT))
#define PLAYER_EXPLODING            (gameData.gameFlags&(1<<PLAYER_EXPLODING_BIT))
#define PLAYER_DEAD                 (gameData.gameFlags&(1<<PLAYER_DEAD_BIT))
#define DEMO_MODE                   (gameData.gameFlags&(1<<DEMO_MODE_BIT))
#define SHOW_PLAYER                 (gameData.gameFlags&(1<<SHOW_PLAYER_BIT))
#define FIRE_LOCKED_OUT             (gameData.gameFlags&(1<<FIRE_LOCKOUT_BIT))
#define OPTION_LOCKED_OUT           (gameData.gameFlags&(1<<OPTION_LOCKOUT_BIT))
#define IS_TWO_PLAYER_GAME          (gameData.gameFlags&(1<<TWO_PLAYER_GAME_BIT))
#define ALIENS_LANDED               (gameData.gameFlags&(1<<ALIENS_LANDED_BIT))
#define NEW_HI                      (gameData.gameFlags&(1<<NEW_HI_BIT))

#define SET_INVADERS_DROPPING       gameData.gameFlags|=(1<<INVADERS_DROPPING_BIT)
#define SET_INVADERS_PAUSED         gameData.gameFlags|=(1<<INVADERS_PAUSED_BIT);gameData.invaderPauseTmr=12;
#define SET_INVADERS_EXPLODING      gameData.gameFlags|=(1<<INVADERS_EXPLODING_BIT)
#define SET_SAUCER_ACTIVE           gameData.gameFlags|=(1<<SAUCER_ACTIVE_BIT)
#define SET_SAUCER_EXPLODING        gameData.gameFlags|=(1<<SAUCER_EXPLODING_BIT)
#define SET_PLAYER_EXPLODING        gameData.gameFlags|=(1<<PLAYER_EXPLODING_BIT)
#define SET_PLAYER_DEAD             gameData.gameFlags|=(1<<PLAYER_DEAD_BIT)
#define SET_DEMO_MODE               gameData.gameFlags|=(1<<DEMO_MODE_BIT)
#define SET_SHOW_PLAYER             gameData.gameFlags|=(1<<SHOW_PLAYER_BIT)
#define SET_FIRE_LOCKED_OUT         gameData.gameFlags|=(1<<FIRE_LOCKOUT_BIT)
#define SET_OPTION_LOCKED_OUT       gameData.gameFlags|=(1<<OPTION_LOCKOUT_BIT)
#define SET_TWO_PLAYER_GAME         gameData.gameFlags|=(1<<TWO_PLAYER_GAME_BIT)
#define SET_ALIENS_LANDED           gameData.gameFlags|=(1<<ALIENS_LANDED_BIT)
#define SET_NEW_HI                  gameData.gameFlags|=(1<<NEW_HI_BIT)

#define UNSET_INVADERS_DROPPING     gameData.gameFlags&=~(1<<INVADERS_DROPPING_BIT)
#define UNSET_INVADERS_PAUSED       gameData.gameFlags&=~(1<<INVADERS_PAUSED_BIT)
#define UNSET_INVADERS_EXPLODING    gameData.gameFlags&=~(1<<INVADERS_EXPLODING_BIT)
#define UNSET_SAUCER_ACTIVE         gameData.gameFlags&=~(1<<SAUCER_ACTIVE_BIT)
#define UNSET_SAUCER_EXPLODING      gameData.gameFlags&=~(1<<SAUCER_EXPLODING_BIT)
#define UNSET_PLAYER_EXPLODING      gameData.gameFlags&=~(1<<PLAYER_EXPLODING_BIT)
#define UNSET_PLAYER_DEAD           gameData.gameFlags&=~(1<<PLAYER_DEAD_BIT)
#define UNSET_DEMO_MODE             gameData.gameFlags&=~(1<<DEMO_MODE_BIT)
#define UNSET_SHOW_PLAYER           gameData.gameFlags&=~(1<<DEMO_MODE_BIT)
#define UNSET_FIRE_LOCKED_OUT       gameData.gameFlags&=~(1<<FIRE_LOCKOUT_BIT)
#define UNSET_OPTION_LOCKED_OUT     gameData.gameFlags&=~(1<<OPTION_LOCKOUT_BIT)
#define UNSET_TWO_PLAYER_GAME       gameData.gameFlags&=~(1<<TWO_PLAYER_GAME_BIT)
#define UNSET_ALIENS_LANDED         gameData.gameFlags&=~(1<<ALIENS_LANDED_BIT)
#define UNSET_NEW_HI                gameData.gameFlags&=~(1<<NEW_HI_BIT)

#define FREQ1                       (1<<CS20)
#define FREQ8                       (1<<CS21)
#define FREQ32                      ((1<<CS20)|(1<<CS21))
#define FREQ64                      (1<<CS22)
#define FREQ128                     ((1<<CS22)|(1<<CS20))
#define FREQ256                     ((1<<CS22)|(1<<CS21))
#define FREQ1024                    ((1<<CS22)|(1<<CS21)|(1<<CS20))

#endif