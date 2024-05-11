#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Scramble\\Scramble\\gameVars.h"
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
    demoEnd,
    chooseGameType,
    playPlayer,
    playing,
    completedLoop,
    lostLife,
    gameOver
} GameState;

typedef struct BulletOb{
    uint8_t *pos;
    uint8_t tileIX;
} BulletOb;

typedef struct FlyingEnemy{
    uint8_t movementIX;
} FlyingEnemy;

typedef struct GameVars{
    uint16_t score;
    uint8_t lives;
    uint8_t level;
    uint8_t loop;
    uint8_t fuelDepletionRate;
    uint8_t ff;
    uint16_t levelAddr;
    uint8_t levelType;
    uint8_t fillBlock[2];
    uint8_t fuel;
    uint8_t autoScoreInc;
    uint8_t playerX;
    uint8_t playerY;
    uint8_t fireLockout;
    uint8_t bombLockout;
    uint8_t bombTmr[2];
    uint8_t alienAddTmr;
    uint8_t colAttr[32];
    uint8_t lastFireballY;
    uint8_t playerAnimCounter;
    uint8_t flag;
    uint8_t audioPos[2];
    const uint8_t *audioPtr[2];
    FlyingEnemy flyingEnemy[MAX_FLYING_ENEMIES];
    BulletOb bulletOb[MAX_BULLETS];
} GameVars;

GameState gameState=idle;
GameVars gV;

uint8_t giu,giv,giw,gix,giy,giz;  // General purpose registers for game

uint8_t credits=0;
uint16_t hiScoreList[10];
const uint8_t *starFieldDataPtr=starFieldData;
uint8_t starFieldCounter=0;

// Animation frames for sprites in the score table screen
const uint8_t scoreTableSpriteDef[6*6] PROGMEM ={
    32,32,32,32,32,32,
    33,34,33,34,33,34,
    13,14,15,13,14,15,
    35,35,35,35,35,35,
    38,37,39,39,37,38,
    36,36,36,36,36,36,
};

// Bytes used to animate player bombs as they fall. This includes the angle and animation frame to replicate original game
// bit 0=increase X
// bit 1=increase Y
// bit 2=increase frame
// bit 3=decrease frame
const uint8_t bombAnim[43] PROGMEM = {
    0, // Does nothing on the first display frame, as it's already positioned and set to the correct animation frame
    2,1,1,1,1, 5,1,1,1,1, 9,1,1,1,1, 5,3,1,1,4, // Initially straight forward, alternating between frames 0 and 1
    3,1,1,3,1,3,7,                              // On frame 2
    1,3,3,3,2,2,3,3,2,3,6,                      // On frame 3
    3,2,3,                                      // On frame 4, after this, it's basically 2's all the way down!
    2,
};

// Bytes used to animate saucers along a path. Pairs of bytes are yDelta,xDelta
const uint8_t saucerAnim[148] PROGMEM = {
    0xFF, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00, 
    0xFE, 0x00, 0xFE, 0x02, 0xFE, 0x00, 0xFE, 0x02, 0xFE, 0x00, 0xFE, 0x02, 0xFE, 0x02, 0xFE, 0x02, 
    0xFE, 0x02, 0x00, 0x02, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
    0x02, 0x02, 0x02, 0x00, 0x02, 0x02, 0x02, 0x00, 0x02, 0x02, 0x02, 0x00, 0x02, 0x02, 0x02, 0x00, 
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x02, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 
    0x02, 0x02, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x02, 0x02, 0x00, 0x02, 0x02, 
    0x02, 0x00, 0x02, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 
    0xFE, 0x02, 0xFE, 0x02, 0xFE, 0x02, 0xFE, 0x02, 0xFE, 0x00, 0xFE, 0x02, 0xFE, 0x00, 0xFE, 0x02, 
    0xFE, 0x00, 0xFE, 0x02, 0xFE, 0x00, 0xFE, 0x02, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00, 
    0xFE, 0x00, 0xFE, 0x00,
};

// Used to animate player explosion
const uint8_t playerExplosionAnim[24] PROGMEM = {
    24,26, 
    24,26,
    28,30, 
    30,28,
    30,28,
    26,24,
    24,26, 
    24,26,
    28,30, 
    30,28,
    30,28,
    26,24,
};

const uint8_t defaultAmbientSound[] PROGMEM = {
    FREQ64, 200,195,190,185,182,180, 1,1
};

const uint8_t saucerAmbientSound[] PROGMEM = {
    FREQ128, 120,116,112,108,104,100,96,92,88,84,80,72,64,56,48,40,32,24,32,40,48,56,64,72,80,84,88,92,96,100,104,108,112,116,120, 1,1
};

const uint8_t bombFallingSound[] PROGMEM = {
    FREQ32, 100,101,102,103,104,105,106,
    107,108,109,110,111,112,113,114,
    115,116,117,118,119,120,121,122,
    123,124,125,126,127,128,129,130,
    131,132,133,134,135,136,137,138,
    139,140,141,142,143,144,145,146,
    147,148,149,150,151,152,153,154,
    155,156,157,158,159,160,161,162,
    163,164,165,166,167,168,169,170,
    171,172,173,174,175,176,177,178,
    179,180,181,182,183,184,185,186,
    187,188,190,191,192,193,194,195,
    196,197,198,199,200,202,204,206,
    208,210,212,214,216,218,220,222,
    1,111
};

const uint8_t bombExplodeSound[] PROGMEM = {
    FREQ8, 2   // 2 = random noise
};

const uint8_t playerExplodeSound[] PROGMEM = {
    FREQ64, 2   // 2 = random noise
};

const uint8_t bulletHitSound[] PROGMEM = {
    FREQ8, 200,198,202,196,204,194,206,192,208,190,210,188,212,186,214,184,216,182,218,0
};

#endif
