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

#ifndef __GAME_VARS
#define __GAME_VARS

#include "gameDefs.h"
#include "engineDefs.h"
#include "gameAssets.h"

typedef enum GameState{
    idle,
    gameInit,
    title,
    demo,
    ranking,
    nameEntry,
    ready,
    playing,
    dying,
    gameOver
} GameState;

GameState gameState=idle;

typedef struct Ghost{
    int16_t x;          // world pixel position of the cell the ghost is in/moving from (a multiple of 8 when it picks a direction)
    int16_t y;
    uint8_t dir;
    uint8_t state;      // GS_*
    uint8_t chain;      // while showing its score: which one (0-7 for 400, 800, 1200 ... 3200)
    uint16_t timer;     // frames until released from the pen
} Ghost;

Ghost ghosts[NUM_GHOSTS];

// Pac-Man
int16_t pacX,pacY;
uint8_t pacDir,pacWant,pacAnim;

// The two halves of the world. Each holds the index of a map in mapData, a bit per cell for "dot still here",
// and a count of remaining dots (including power pellets)
uint8_t halfMap[2];
uint8_t dotBits[2][(MAP_SIZE+7)/8];
uint8_t dotsLeft[2];
uint8_t mapSeq;         // next map to use when a half is regenerated
uint8_t clearedMask;    // bit per half that has been cleared and is waiting for the fruit to be eaten
uint8_t pendingWipe;    // bit per half that is waiting for its turn to be replaced

// A half-map being replaced one column at a time, moving away from the centre of the world
uint8_t wipeHalf=255;   // which half, or 255 for none
uint8_t wipeProg;       // number of columns already replaced
uint8_t wipeNewMap;

uint8_t fruitActive;
uint8_t fruitCX,fruitCY;
uint8_t fruitType;      // which of the icons (0 = cherry ... 21 = crown) is the current one
uint8_t fruitsEaten;    // how many have been eaten this game - the next one is the next icon

// Camera: world pixel x of the left edge of the screen (wraps at WORLD_PX_W) and the first world cell column held in
// screen RAM column 0
int16_t camX;
uint8_t viewTC;

uint32_t score;
uint8_t lives;
uint16_t framesLeft;
uint16_t scareTimer;
uint8_t ghostChain;
uint8_t dyingTimer;     // counts up while dying
uint8_t tick;
uint16_t rng=0xACE1;
uint8_t hudDirty;
uint8_t freezeTimer;

// The score for an eaten ghost, flying up to the score display: frames left, which score it is (0-7), and where it started
uint8_t scoreFlyTimer,scoreFlyIdx,scoreFlyX,scoreFlyY;

// Score trail behind Pac-Man: a number shown where a dot was eaten, until its timer runs out
typedef struct TrailDot{
    uint8_t cx;
    uint8_t cy;
    uint8_t timer;      // frames left, 0 = slot free
    uint8_t value;      // points / 10
} TrailDot;
TrailDot trail[TRAIL_SLOTS];
uint16_t dotsEaten;     // since the last death

// Start-of-game music position
uint8_t introIx,introTimer;

// Sound effect on channel 0
uint8_t sfxType,sfxTimer,sfxFrame,sfxToggle,sfxLow;


// Which thing the scrolling part of the tilemap is currently showing, and how big its world is
uint8_t scrollMode=SM_GAME;
uint8_t scrollW=WORLD_W;
uint8_t scrollRows=WORLD_H;

// Title screen and attract mode
uint8_t attractPhase;
uint16_t attractTimer;
uint8_t bandScene;
uint8_t demoMode;
uint8_t screenAfterRank;
uint8_t rankBlank=255;   // high score entry that is currently blinked off (255 = none)
uint8_t rankFrame;       // frames since the blink last moved on

// Wave that replaces a half map: columns the wave has turned into the new maze are shown "filled in" until the
// ripple that follows it turns them back
uint8_t wipeRipple;
uint8_t wipeRippling;

// Name entry
uint8_t entryName[3];
uint8_t entryPos;
uint8_t entryRank;

#endif
