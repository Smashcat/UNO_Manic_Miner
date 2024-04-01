/*
 
 This file is part of Manic Miner for Arduino UNO.
 Copyright (C) 2024 Scott Porter

 Manic Miner for Arduino UNO is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by the Free Software 
 Foundation, either version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the  GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>

 */

#ifndef __GAME_VARS
#define __GAME_VARS

#include "gameDefs.h"

/**
 * @brief Used to store information about missiles/bombs
 * 
 */
typedef struct Missile{
    /**
     * @brief Direction and speed of missile in Y axis (missiles do not move in X axis) If dirY is zero, missile/bomb is not active
     * 
     */
    int8_t  dirY;
    /**
     * @brief Current column index pointer for squiggly and plunger bombs to look up next column to use
     * 
     */
    uint8_t colIX;
    /**
     * @brief The base animation frame for missile (only used for bombs as player missile only has one frame)
     * 
     */
    uint8_t baseDefIX;
    /**
     * @brief The exploding animation frame to use (player missile uses different one to alien bombs)
     * 
     */
    uint8_t expDefIX;
    /**
     * @brief If not zero, exploding frame shown, and when this returns to zero, missile/bomb is reset for reuse
     * 
     */
    uint8_t expTmr;
} Missile;

typedef enum GameState{
    idle,
    gameInit,
    title,
    demo,
    pricing,
    chooseGameType,
    playPlayer,
    playing,
    levelClear,
    gameOver
} GameState;

typedef struct GameData{

    // If bits are 1, invader is on screen. LSb is left-most alien, array index 0 is bottom.
    // So at level start, each entry in the array will be 0x7F (0b0000011111111111)
    uint16_t invaderBitmap[INVADER_ROWS];

    // The bit in the bitmap array being updated. This runs from 0-55, checking each bit. If a bit is zero, it's skipped until it finds a 1 to update
    // each frame. So there is always exactly 1 alien updating each frame, unless all are dead.
    uint8_t invaderUpdateBit;

    // This is the position of the reference alien on the tilemap. This alien may or may not be alive, but all others move in relation to this.
    int16_t cursorX;
    uint8_t cursorY;

    // Number of invaders drawn initially - so they gradually appear at start.
    uint8_t revealedInvaders;

    // Invaders left on screen this level
    uint8_t remainingInvaders;

    // Direction of movement for invaders, either 2 or -2
    int8_t invaderDir;

    // Current offset/frame for invaders being moved
    int8_t invaderOffset;

    // Timer to pause invaders when one is hit
    uint8_t invaderPauseTmr;
    
    // The current minimum and maximum x positions for the reference alien. This allows remaining aliens to walk to edges of screen
    int16_t cursorXMin;
    int16_t cursorXMax;

    // Various logic bits for the gameplay - see gameDefs.h 
    uint16_t gameFlags;

    // Direction of saucer, either 0, 2 or -2, depends on number of shots fired mod 2 when it appears
    // If 0, the saucer is not on screen
    // Saucer moves by 2 pixels every 3 frames
    int8_t saucerDir;

    // X position of saucer if it's on screen
    uint8_t saucerX;

    uint8_t currentPlayerIX;

    // Player's position on screen
    uint8_t playerX;

    // Used for explosion animation
    uint8_t playerTmr;

    // shot timer used to show player missile exploding against top of screen before resetting it, if it misses everything else
    Missile missile[4];

    // How many times player has shot, mod 15 - this is used to pick the saucer score, and also which side the saucer appears from (even=right,odd=left)
    // So seasoned players will count shots - first shooting 22 (doesn't matter if they hit or miss), then waiting for the saucer, so the 23rd shot hits
    // it for a guaranteed 300. Then firing 14 shots and using the 15th for the next saucer, then 14, then 15th for saucer, repeated for 300 points each 
    // time. Nothing is random in this game.
    uint8_t shotCnt;

    // Timer before next saucer appears (initially after 35 seconds, then approx every 20 seconds until only 5 invaders left) - note saucer cannot appear 
    // if squiggly bomb is on screen. This is reloaded from framesToSaucer constant
    uint16_t saucerTmr;

    uint8_t saucerScore;

    // Timer before bomb of each type can drop
    uint8_t reloadRate;

    // Current amount of frames before another bomb can be dropped - this is reset to the current reloadRate when a bomb is dropped
    uint8_t movTmr;

    // Current alien missile speed. This is usually 4, but changes to 5 when 8 or less aliens left
    uint8_t alienMissileSpeed;

    // Counts from 0-2, then back to 0. This controls which alien bomb is active this frame. Only active bombs can be fired or moved per frame.
    uint8_t bombIX;

    // Lives remaining
    uint8_t lives[2];

    // Current level (start Y position of aliens based on this, but they wrap at 8, back to 1)
    uint8_t level[2];

    // Player scores
    uint16_t score[2];

    // Unused credits
    uint8_t credit;

    // Just used to choose between static title/pricing screens and animated versions (upside down Y, and CCOIN versions)
    uint8_t demoModeCount=0;

    // Just used to control player ship in demo mode
    uint8_t demoModeTmr;
    uint8_t demoModePlayerTargetX;
    
    // Global game timer, currently only used to decide when player ship appears on screen at level start
    uint16_t gameTmr;

    // Which audio beat will be played next as invader rank moves
    uint8_t beatIX;

    // Index into audioDelay table that determines the delay between beats, based on the number of aliens on screen
    uint8_t beatDelayIX;

    // Timer set by lookup from audioDelay, until the next beat is played
    uint8_t beatTmr;

    // Timer used to cut of beat after X frames so it's not constant
    uint8_t beatCutTmr;

} RankData;

GameState gameState=idle;
GameData gameData;

uint8_t giu,giv,giw,gix,giy,giz;  // General purpose registers for game

const uint8_t fontIX=180;  // From the comments in gameAssets we know this is where the font data begins...
const uint8_t blankTile=fontIX+38;
#endif
