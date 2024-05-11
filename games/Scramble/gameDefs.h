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

// Maximum number of bullets player can have on screen at one time (default is 4 to match original game but it looks cooler with more :) )
#define MAX_BULLETS                 4

// Slowest rate at which fuel will deplete during game (this is the initial value at the start of the game)
#define SLOWEST_FUEL_DEPLETION_RATE 21

// Fastest rate at which fuel will deplete during game (this is the value used from the third loop around the stages onwards)
#define FASTEST_FUEL_DEPLETION_RATE 13

// Used all over the place to clear the tilemap. This is the index that defines a blank tile.
#define BLANK_TILE                  16

// Maximum number of flying enemies on screen at any one time. The original game specifies 4, and
// there is no point changing it really due to how the timers work. It's just here as a reference.
#define MAX_FLYING_ENEMIES          4

#define FLAG_BIT_GAME_OVER          0
#define FLAG_BIT_LOST_LIFE          1
#define FLAG_BIT_PLAYER_DEAD        2
#define FLAG_BIT_IS_PAUSED          3
#define FLAG_BIT_DEMO_MODE          4
#define FLAG_BIT_HIT_BASE           5
#define FLAG_BIT_OUT_OF_FUEL        6

#define SET_GAME_OVER               gV.flag|=(1<<FLAG_BIT_GAME_OVER)
#define SET_LOST_LIFE               gV.flag|=(1<<FLAG_BIT_LOST_LIFE)
#define SET_PLAYER_DEAD             gV.flag|=(1<<FLAG_BIT_PLAYER_DEAD);gV.playerAnimCounter=0;
#define SET_IS_PAUSED               gV.flag|=(1<<FLAG_BIT_IS_PAUSED)
#define SET_DEMO_MODE               gV.flag|=(1<<FLAG_BIT_DEMO_MODE)
#define SET_HIT_BASE                gV.flag|=(1<<FLAG_BIT_HIT_BASE)
#define SET_OUT_OF_FUEL             gV.flag|=(1<<FLAG_BIT_OUT_OF_FUEL)

#define CLEAR_GAME_OVER             gV.flag&=~(1<<FLAG_BIT_GAME_OVER)
#define CLEAR_LOST_LIFE             gV.flag&=~(1<<FLAG_BIT_LOST_LIFE)
#define CLEAR_PLAYER_DEAD           gV.flag&=~(1<<FLAG_BIT_PLAYER_DEAD)
#define CLEAR_IS_PAUSED             gV.flag&=~(1<<FLAG_BIT_IS_PAUSED)
#define CLEAR_DEMO_MODE             gV.flag&=~(1<<FLAG_BIT_DEMO_MODE)
#define CLEAR_HIT_BASE              gV.flag&=~(1<<FLAG_BIT_HIT_BASE)
#define CLEAR_OUT_OF_FUEL           gV.flag&=~(1<<FLAG_BIT_OUT_OF_FUEL)

#define IS_GAME_OVER                (gV.flag&(1<<FLAG_BIT_GAME_OVER))
#define IS_LOST_LIFE                (gV.flag&(1<<FLAG_BIT_LOST_LIFE))
#define IS_PLAYER_DEAD              (gV.flag&(1<<FLAG_BIT_PLAYER_DEAD))
#define IS_PAUSED                   (gV.flag&(1<<FLAG_BIT_IS_PAUSED))
#define IS_DEMO_MODE                (gV.flag&(1<<FLAG_BIT_DEMO_MODE))
#define IS_HIT_BASE                 (gV.flag&(1<<FLAG_BIT_HIT_BASE))
#define IS_OUT_OF_FUEL              (gV.flag&(1<<FLAG_BIT_OUT_OF_FUEL))

#define NOT_GAME_OVER                ((gV.flag&(1<<FLAG_BIT_GAME_OVER))==0)
#define NOT_LOST_LIFE                ((gV.flag&(1<<FLAG_BIT_LOST_LIFE))==0)
#define NOT_PLAYER_DEAD              ((gV.flag&(1<<FLAG_BIT_PLAYER_DEAD))==0)
#define NOT_PAUSED                   ((gV.flag&(1<<FLAG_BIT_IS_PAUSED))==0)
#define NOT_DEMO_MODE                ((gV.flag&(1<<FLAG_BIT_DEMO_MODE))==0)
#define NOT_HIT_BASE                 ((gV.flag&(1<<FLAG_BIT_HIT_BASE))==0)
#define NOT_OUT_OF_FUEL              ((gV.flag&(1<<FLAG_BIT_OUT_OF_FUEL))==0)

#endif