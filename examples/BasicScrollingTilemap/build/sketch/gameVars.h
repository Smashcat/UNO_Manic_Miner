#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\examples\\BasicScrollingTilemap\\gameVars.h"
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


GameState gameState=idle;

uint8_t giu,giv,giw,gix,giy,giz;  // General purpose registers for game

const uint8_t fontIX=180;  // From the comments in gameAssets we know this is where the font data begins...
const uint8_t blankTile=fontIX+38;

#endif
