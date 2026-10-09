/*

 This file is part of the Arduino UNO game engine.
 Copyright (C) 2024 Scott Porter

 The Arduino UNO game engine is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the  GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>

 */

#ifndef __GAME_DEFS
#define __GAME_DEFS

// Used all over the place to clear the tilemap. This is the index that defines a blank tile.
#define BLANK_TILE                  16

// ---- Game rules ---------------------------------------------------------------------------------------------

// Length of a game in seconds. The game ends when this runs out, or all lives are lost.
#define GAME_SECONDS                300
#define START_LIVES                 3
#define MAX_LIVES                   5      // (the HUD has room for this many life icons)
#define LIFE_CHOMP_FRAMES           8      // the life icons at the bottom change mouth shape this often

// An extra life is awarded every EXTRA_LIFE_SCORE points, with a jingle of dings on the ghost sound channel
#define EXTRA_LIFE_SCORE            20000
#define EXTRA_LIFE_DING_FRAMES      6       // frames for each ding (the last frame of each is a gap)
#define EXTRA_LIFE_DINGS            5       // dings in a group
#define EXTRA_LIFE_FRAMES           (EXTRA_LIFE_DING_FRAMES*EXTRA_LIFE_DINGS)
#define EXTRA_LIFE_HOLD             2       // the very last note is held on for this many extra frames
#define EXTRA_LIFE_REPEATS          2      // how many groups play (set to 1 once the jingle has been tuned)
#define EXTRA_LIFE_TOTAL            ((uint16_t)EXTRA_LIFE_FRAMES*EXTRA_LIFE_REPEATS+EXTRA_LIFE_HOLD)   // length of the whole jingle

// Frames (50 per second) that ghosts stay frightened after a power pellet at the start of a game. Each fruit eaten (each half maze
// replaced) shortens it by SCARE_REDUCTION, down to a minimum of SCARE_MIN_FRAMES
#define SCARE_FRAMES                400      // 8 seconds
#define SCARE_REDUCTION             25       // half a second less each time
#define SCARE_MIN_FRAMES            225      // but never less than 4.5 seconds


#define SCORE_DOT                   10
#define SCORE_PELLET                50
#define SCORE_GHOST                 400   // 400 for the first ghost eaten, then 800, 1200 ... up to 3200 from the eighth on. The chain carries on if another pellet is eaten before the effect wears off

// An eaten ghost turns into its score, which flies up to the score display over this many frames and is then added
#define SCORE_FLIGHT                24
#define SCORE_TARGET_X              72
#define SCORE_TARGET_Y              8

// The points for a dot start at 10 and go up by 10 every DOT_VALUE_STEP dots eaten, to a maximum of 50. Dying resets it.
// Each dot eaten leaves its value showing for a short while (the trail behind Pac-Man), up to TRAIL_SLOTS at a time
#define DOT_VALUE_STEP              60
#define TRAIL_SLOTS                 8
#define TRAIL_FRAMES                32

// Frames the whole game freezes for when a ghost is eaten, to give the hit some weight
#define GHOST_EAT_FREEZE            6

// Frames between each column of the half-map being replaced after the fruit is eaten
#define WIPE_FRAMES                 2

// ---- World layout -------------------------------------------------------------------------------------------
// The world is a left half-map, a fixed centre strip and a right half-map, 47x23 cells. A cell is one 8x8 tile.
// It wraps around both horizontally and vertically. The maze is only 184 pixels tall so it never scrolls vertically.

#define WORLD_W                     (MAP_W+CENTER_W+MAP_W)
#define WORLD_H                     (MAP_H)
#define WORLD_PX_W                  (WORLD_W*8)
#define WORLD_PX_H                  (WORLD_H*8)

// First column (in world cells) of the right half, and the sprite is drawn this many pixels up/left of the cell origin
// so that it is centred on the cell
#define RIGHT_HALF_X                (MAP_W+CENTER_W)
#define SPRITE_OFFSET               4

// Four tile rows at the top of the screen are static (the HUD: two rows of double height text with a blank row each side),
// then 24 scrolling rows of which the maze uses 23.
// There are three static rows below that too (READY, and the lives which are two rows high)
#define HUD_ROWS                    4
#define VIEW_TOP                    (HUD_ROWS*8)
#define VIEW_BOTTOM                 (VIEW_TOP+(24*8))
#define LIVES_ROW                   5      // the 16x16 life icons use this row and the next

// Pac-Man's start cell (in the corridor under the ghost pen)
#define START_CX                    23
#define START_CY                    14


// ---- Ghost pen (the box in the centre of the world) ----------------------------------------------------------
// Ghosts bounce around inside the box (pixel positions below), leave through the gate, and eyes return the same way
#define PEN_X0                      168     // left-most x of a ghost inside the box
#define PEN_X1                      200
#define PEN_Y0                      80      // top and bottom of the box interior
#define PEN_Y1                      96
#define GATE_X                      184     // x of the middle of the gate
#define EXIT_CX                     23      // the cell just above the gate that ghosts leave to and eyes return to
#define EXIT_CY                     8
#define EXIT_Y                      (EXIT_CY*8)

// Scatter: for this many frames in every CHASE_CYCLE frames the ghosts head for their own corner
#define CHASE_CYCLE                 1500
#define SCATTER_FRAMES              300

// ---- Sprites ------------------------------------------------------------------------------------------------
#define NUM_GHOSTS                  4
#define SPRITE_FRUIT                0
#define SPRITE_GHOST0               1
#define SPRITE_PAC                  5
#define SPRITE_SCORE                6      // the number that flies up to the score when a ghost is eaten

// ---- Ghost states -------------------------------------------------------------------------------------------
#define GS_NONE                     0
#define GS_PEN                      1   // bouncing around inside the centre box, waiting to be released
#define GS_AWAKE                    2
#define GS_SCARED                   3
#define GS_EYES                     4
#define GS_LEAVING                  5   // on its way out through the gate
#define GS_ENTER                    6   // eyes going back in through the gate
#define GS_SCORE                    7   // just eaten: showing its score, which is flying up to the score display

// Directions are clockwise from up
#define DIR_UP                      0
#define DIR_RIGHT                   1
#define DIR_DOWN                    2
#define DIR_LEFT                    3
#define DIR_NONE                    255


// ---- Dying ----------------------------------------------------------------------------------------------------
// Pac-Man gets hit: the game holds for a moment with everything visible, the ghosts vanish, and then he folds up
// like the arcade game while the death sound plays
#define DEATH_PAUSE                 13      // frames (about 250ms) of nothing happening after the hit
#define DEATH_ANIM_FRAMES           80      // length of the death sound and the animation
#define DEATH_FRAME_LEN             (DEATH_ANIM_FRAMES/10)
#define DEATH_END_PAUSE             16

// ---- Scrolling modes -------------------------------------------------------------------------------------------
// The tilemap scrolls horizontally over the maze in the game, the characters band on the title screen, and the
// high score page when it slides on and off
#define SM_GAME                     0
#define SM_BAND                     1
#define SM_RANK                     2
#define BAND_W                      64
#define RANK_W                      64

// Title screen layout: the logo and the stripes are static, the band between them scrolls
#define BAND_TOP                    112
#define BAND_BOTTOM                 168
#define BAND_ROWS                   7
#define BAND_SCREEN_ROW             14      // first screen tile row of the band

// The ghost siren sweeps up and down faster as the game goes on: phase steps (out of 256) per frame is
// SIREN_SPEED_BASE plus the number of mazes used so far, up to SIREN_SPEED_MAX of them
#define SIREN_SPEED_BASE            12     // 12 is about 0.85 seconds per "whoo" at the start, 22 about 0.45 at the fastest
#define SIREN_SPEED_MAX             10

// How long the demo runs before it gives way to the high scores (frames)
#define DEMO_FRAMES                 500
// The parade round the high score table (Pac-Man, then the four ghosts) runs along a path in the margins round it, at this many
// pixels a frame, with this many pixels between each character. It ends when the last ghost has gone off the right hand side
#define PARADE_SPEED                3
#define PARADE_SPACING              24
#define PARADE_LEFT                 8       // x of the run up the left side, and of the run down the right side
#define PARADE_RIGHT                206
#define PARADE_TOP                  26       // y of the run across between the title and the table, and along the bottom
#define PARADE_BOTTOM               208
#define PARADE_END                  (((256-PARADE_LEFT)+(PARADE_BOTTOM-PARADE_TOP)+(PARADE_RIGHT-PARADE_LEFT)+(PARADE_BOTTOM-PARADE_TOP)+(256-PARADE_RIGHT))+(PARADE_SPACING*4))
// Testing: 1 starts the game on the high score page (it goes on to the title screen afterwards as usual)
#define START_ON_RANKING            0
#define RANK_BLINK_FRAMES           3       // each high score entry blinks off for this many frames (50 per second, so 5 = 100ms) in turn
#define RESET_MESSAGE_FRAMES        100      // "HIGH SCORES RESET" (shown when they are cleared at start-up) stays up for this long
#define GAMEOVER_FRAMES             250      // (must stay under 255 - it is counted by "tick")

// GAME OVER: the two words glide in from opposite sides (GAME from the left, OVER from the right), taking GAMEOVER_GLIDE_FRAMES
// to ease into place, with OVER starting GAMEOVER_WORD_DELAY frames after GAME. Both bob on a sine wave all the time (64 steps to
// a cycle), half a cycle apart, moving on one step every GAMEOVER_WAVE_FRAMES frames: 1 is a smooth wave about 1.3 seconds long
#define GAMEOVER_GLIDE_FRAMES       48
#define GAMEOVER_WORD_DELAY         0
#define GAMEOVER_WAVE_FRAMES        2

// ---- High scores -----------------------------------------------------------------------------------------------
#define NUM_SCORES                  10
#define EE_BASE                     16      // EEPROM address of the table (0-15 are used by the engine)
#define EE_MAGIC                    0xA6

#endif
