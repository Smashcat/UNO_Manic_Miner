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

#ifndef __ENGINE_DEFS
#define __ENGINE_DEFS

// If using NES controller, set this to 1, otherwise 0 for normal buttons. If you want to connect a controller to the
// header pins, then use the defines beginning with "NES_" below. These are all on the B port (add 8 to each one to find its "D" number on the UNO board)
#define USE_CONTROLLER          1

// Currently the engine supports up to 9 16x16 sprites on screen at once. With up to 7 in any one line, which is, again, ideal for Manic Miner.
// Sprite 8 (the last one) is a special player sprite. It also does pixel-based collision detection with the other sprites and sets or clears the collision flag
// each frame.
#define NUM_SPRITES             9

// Can be longer than the raster line if horizontal scrolling needs to be really fast. Just allows for move offscreen pixels to be written
#define BYTES_PER_BUFFER_LINE   32

// Horizontal pixels per tile - usually 8 (tiles are always 8 pixels wide)
#define PIXELS_PER_CHARACTER    8

// Vertical rows in tiles. Default is 32, for 256 pixels vertically
#define CHARACTER_ROWS          32

// Nothing else should be changed from here down, unless you want to change input pins, or modify code etc.

#define MUTE_PWM_0            DDRD&=~(1<<6)
#define UNMUTE_PWM_0          DDRD|=(1<<6)

#define MUTE_PWM_1            DDRB&=~(1<<3)
#define UNMUTE_PWM_1          DDRB|=(1<<3)

#if USE_CONTROLLER

#define BUTTON_RIGHT          (1<<0)
#define BUTTON_LEFT           (1<<1)
#define BUTTON_DOWN           (1<<2)
#define BUTTON_UP             (1<<3)
#define BUTTON_START          (1<<4)
#define BUTTON_OPTION         (1<<5)
#define BUTTON_B              (1<<6)
#define BUTTON_A              (1<<7)

#define NES_DATA              5
#define NES_LATCH             4
#define NES_CLOCK             2

#else

#define BUTTON_B              (1<<5)
#define BUTTON_A              (1<<4)
#define BUTTON_DOWN           (1<<3)
#define BUTTON_UP             (1<<2)
#define BUTTON_RIGHT          (1<<1)
#define BUTTON_LEFT           (1<<0)
#define BUTTON_START          (1<<6)
#define BUTTON_OPTION         (1<<7)

#endif

#define PRESSING_START        ((keyState.current&BUTTON_START)==0)
#define PRESSING_OPTION       ((keyState.current&BUTTON_OPTION)==0)
#define PRESSING_A            ((keyState.current&BUTTON_A)==0)
#define PRESSING_B            ((keyState.current&BUTTON_B)==0)
#define PRESSING_LEFT         ((keyState.current&BUTTON_LEFT)==0)
#define PRESSING_RIGHT        ((keyState.current&BUTTON_RIGHT)==0)
#define PRESSING_UP           ((keyState.current&BUTTON_UP)==0)
#define PRESSING_DOWN         ((keyState.current&BUTTON_DOWN)==0)
#define NOTHING_PRESSED       (keyState.current==0xff)

// If zero, no horizontal scrolling code is included for the tilemap.
#define USE_MASK        1

// If any of these are 0, then sprite will overwrite sprites under it instead of ORing with them. Improves performance substantially for sprites that don't overlap!
// Sprites are always ORd with the background tiles no matter what this is set to
#define OR_SPRITE0      1
#define OR_SPRITE1      1
#define OR_SPRITE2      1
#define OR_SPRITE3      1
#define OR_SPRITE4      1
#define OR_SPRITE5      1
#define OR_SPRITE6      1
#define OR_SPRITE7      1

// If set to 0, then sprite can only move in the vertical axis, or if moving horizontally, can only be 8 pixels wide, or will clip as it moves. Improves performance 
// for games where some sprites are only 8 pixels or less wide
#define V_SPRITE0       1
#define V_SPRITE1       1
#define V_SPRITE2       1
#define V_SPRITE3       1
#define V_SPRITE4       1
#define V_SPRITE5       1
#define V_SPRITE6       1
#define V_SPRITE7       1

// Always 32 for current renderer for 256 pixels horizontally
#define BYTES_PER_RASTER 32

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
// and when it's stopped. 303 maximum to be on-screen on 7-inch monitor - you can subtract 1 here to hide the bottom row of tiles for smoother vertical scrolling
#define BOTTOM_EDGE (TOP_EDGE + (CHARACTER_ROWS-1) * PIXELS_PER_CHARACTER)

#define PROCESSED_SPRITE_OFFSET     200

// Audio base frequencies
#define FREQ1                       (1<<CS20)
#define FREQ8                       (1<<CS21)
#define FREQ32                      ((1<<CS20)|(1<<CS21))
#define FREQ64                      (1<<CS22)
#define FREQ128                     ((1<<CS22)|(1<<CS20))
#define FREQ256                     ((1<<CS22)|(1<<CS21))
#define FREQ1024                    ((1<<CS22)|(1<<CS21)|(1<<CS20))


#endif
