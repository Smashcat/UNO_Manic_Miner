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

#ifndef __MM_TYPES
#define __MM_TYPES

#define START_LEVEL           0
#define START_LIVES           3
//#define DISABLE_SPRITE_COLLISION
//#define DISABLE_NASTY_COLLISION
//#define DISABLE_COLLECT_PICKUPS


#define BUTTON_B              (1<<0)
#define BUTTON_A              (1<<1)
#define BUTTON_DOWN           (1<<2)
#define BUTTON_UP             (1<<3)
#define BUTTON_RIGHT          (1<<4)
#define BUTTON_LEFT           (1<<5)
#define BUTTON_START          (1<<6)
#define BUTTON_OPTION         (1<<7)

//bit 0 = can go right, bit 1 = can go left, bit 2 = in air, bit 3 = can jump
#define CAN_MOVE_RIGHT        (1<<0)
#define CAN_MOVE_LEFT         (1<<1)
#define NOT_IN_AIR            (1<<2)
#define CLEAR_ABOVE           (1<<3)
#define BLOCK_UNDER           (1<<4)
#define ON_CONVEYOR           (1<<5)

#define TILE_TYPE_BACKGROUND  0
#define TILE_TYPE_FLOOR       1
#define TILE_TYPE_CRUMBLING   2
#define TILE_TYPE_WALL        3
#define TILE_TYPE_CONVEYOR    4
#define TILE_TYPE_NASTY1      5
#define TILE_TYPE_NASTY2      6
#define TILE_TYPE_SPARE       7
#define TILE_TYPE_PICKUP      8
#define TILE_TYPE_PORTAL      9

#define GF_SHOWING_AIR        (1<<0)
#define GF_LEVEL_COMPLETE     (1<<1)
#define GF_JUMP_LOCKOUT       (1<<2)
#define GF_PORTAL_READY       (1<<3)
#define GF_PICKUP_SND         (1<<4)
#define GF_EUGENE_ACTIVE      (1<<5)
#define GF_KONG_SWITCH_1      (1<<6)
#define GF_KONG_SWITCH_2      (1<<7)

#define GF2_WANT_MOVE_RIGHT   (1<<0)
#define GF2_CHANGE_DIR        (1<<1)
#define GF2_ANIMATE_TILES     (1<<2)
#define GF2_ENABLE_MUSIC      (1<<3)

#define WF_HAS_KONG           (1<<7)
#define WF_HAS_EUGENE         (1<<6)
#define WF_HAS_VERT_GUARDIANS (1<<5)
#define WF_FACING_LEFT        (1<<4)

#define LF_CONVEYOR_RIGHT     (1<<5)
#define LF_SKYLAB_LEVEL       (1<<6)
#define LF_SOLAR_POWER_LEVEL  (1<<7)

#define PRESSING_JUMP         ((keyState.current&BUTTON_B)==0)
#define PRESSING_LEFT         ((keyState.current&BUTTON_LEFT)==0)
#define PRESSING_RIGHT        ((keyState.current&BUTTON_RIGHT)==0)
#define PRESSING_UP           ((keyState.current&BUTTON_UP)==0)
#define PRESSING_DOWN         ((keyState.current&BUTTON_DOWN)==0)

#define WANT_TO_MOVE_RIGHT    (gameFlags2&GF2_WANT_MOVE_RIGHT)
#define WANT_TO_MOVE_LEFT     ((gameFlags2&GF2_WANT_MOVE_RIGHT)==0)
#define DID_CHANGE_DIR        (gameFlags2&GF2_CHANGE_DIR)

#define SET_MOVE_RIGHT        gameFlags2|=GF2_WANT_MOVE_RIGHT
#define SET_MOVE_LEFT         gameFlags2&=~GF2_WANT_MOVE_RIGHT
#define SET_CHANGE_DIR        gameFlags2|=GF2_CHANGE_DIR

#define SET_TILE_ANIMATION_ON gameFlags2|=GF2_ANIMATE_TILES
#define SET_TILE_ANIMATION_OFF gameFlags2&=~GF2_ANIMATE_TILES

#define SET_MUSIC_ON          gameFlags2|=GF2_ENABLE_MUSIC
#define SET_MUSIC_OFF         gameFlags2&=~GF2_ENABLE_MUSIC

#define WILLY_NOT_JUMPING     (willyFlags[0]==0)
#define WILLY_FACING_LEFT     (willyFlags[1]&WF_FACING_LEFT)
#define WILLY_FACING_RIGHT    ((willyFlags[1]&WF_FACING_LEFT)==0)

#define WILLY_SET_FACE_LEFT   willyFlags[1]|=WF_FACING_LEFT
#define WILLY_SET_FACE_RIGHT  willyFlags[1]&=~WF_FACING_LEFT

#define CONVEYOR_GOING_LEFT   ((levelFlags&LF_CONVEYOR_RIGHT)==0)
#define CONVEYOR_GOING_RIGHT  (levelFlags&LF_CONVEYOR_RIGHT)

#define IS_SKYLAB_LEVEL       (levelFlags&LF_SKYLAB_LEVEL)
#define IS_SOLAR_POWER_LEVEL  (levelFlags&LF_SOLAR_POWER_LEVEL)

#define DO_TILE_ANIMATION     (gameFlags2&GF2_ANIMATE_TILES)
#define IS_MUSIC_ON           (gameFlags2&GF2_ENABLE_MUSIC)

#define MUTE_PWM_1            DDRD&=~(1<<6)
#define UNMUTE_PWM_1          DDRD|=(1<<6)

#define MUTE_PWM_2            DDRB&=~(1<<3)
#define UNMUTE_PWM_2          DDRB|=(1<<3)



// Engine related defines below

// If zero, no horizontal scrolling code is included for the tilemap. Manic Miner doesn't use it, but future games will :)
#define USE_MASK        0

// If any of these are 0, then sprite will overwrite sprites under it instead of ORing with them. Improves performance quite a bit for sprites that don't overlap!
// For example, in Manic Miner, vertical sprites NEVER overlap, and as they are the first ones, we can turn it off for them. The sprite at index 0 doesn't need to OR
// and the sprite at index 8 is the special player sprite that checks for collisions, so has to OR
#define OR_SPRITE1      0
#define OR_SPRITE2      0
#define OR_SPRITE3      0
#define OR_SPRITE4      1
#define OR_SPRITE5      1
#define OR_SPRITE6      1
#define OR_SPRITE7      1

// If set to 0, then sprite can only move in the vertical axis, or if moving horizontally, can only be 8 pixels wide, or will clip as it moves. This is because
// the 16x16 sprites can span 24x16 pixels when moving horizontally - requiring 3 bytes per line-buffer entry. If the sprite will not be moving horizontally,
// this can be reduced to only 2 bytes for these sprites, saving some time and a few bytes of program memory. Generally you would leave these all set to 1, but
// I know that in Manic Miner the first 4 sprites are always only moving vertically, except for the "Congratulations!" screen if you finish the 20th level, where
// the code ensures these first 4 are always 8-bit aligned in the x-axis during the animation :)
#define V_SPRITE0       0
#define V_SPRITE1       0
#define V_SPRITE2       0
#define V_SPRITE3       0
#define V_SPRITE4       1
#define V_SPRITE5       1
#define V_SPRITE6       1
#define V_SPRITE7       1

// Always 32 for current renderer for 256 pixels horizontally
#define BYTES_PER_RASTER 32

// Can be longer than the raster line if horizontal scrolling needs to be really fast. Just allows for move offscreen pixels to be written
#define BYTES_PER_BUFFER_LINE 32
// Horizontal pixels per tile - usually 8 (tiles are always 8 pixels wide)
#define PIXELS_PER_CHARACTER 8
// Vertical rows in tiles. Default is 32, for 256 pixels vertically - Manic Miner actually hides the bottom row of tiles by starting the vblank after the 31st row
#define CHARACTER_ROWS 32
// Currently the engine supports up to 9 16x16 sprites on screen at once. With up to 7 in any one line, which is, again, ideal for Manic Miner.
// Sprite 8 (the last one) is a special player sprite. It also does pixel-based collision detection with the other sprites and sets or clears the collision flag
// each frame.
#define NUM_SPRITES 9

// Pin used to prevent the UART from outputting a pixel as it is enabled at the start of the scanline. Unfortunately there's not really any other way to do this
#define PIN_SUPPRESS 4

// Pin used to signal the v/h-sync period. Do not change. It's actually PORTB, bit 1
#define PIN_SYNC 9

// There is one extra tile in x and y to allow for smooth scrolling without glitches
#define SCREEN_RAM_SIZE ((BYTES_PER_BUFFER_LINE) * (CHARACTER_ROWS))

// time into scanline when pixel pumper awakes (but it takes time to get going because of the interrupt handler etc.
// Units: us * 16
#define LEFT_EDGE 212
// scan line at which pixel pumper is enabled. 30 minimum to be on-screen on 7-inch monitor
#define TOP_EDGE 48
// and when it's stopped. 303 maximum to be on-screen on 7-inch monitor - subtracts 1 here to hide the bottom row of tiles for smoother vertical scrolling
#define BOTTOM_EDGE (TOP_EDGE + (CHARACTER_ROWS-1) * PIXELS_PER_CHARACTER - 1)
#endif
