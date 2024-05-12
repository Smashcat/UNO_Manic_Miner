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

#endif
