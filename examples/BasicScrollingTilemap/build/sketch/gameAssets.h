#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\examples\\BasicScrollingTilemap\\gameAssets.h"
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

#ifndef GAME_ASSETS_H_
#define GAME_ASSETS_H_
#include <avr/pgmspace.h>

// Text to scroll across top of screen
const uint8_t scrollText[256] PROGMEM = {0xD3, 0xC7, 0xC8, 0xD2, 0x00, 0xC8, 0xD2, 0x00, 0xC0, 0xCD, 0x00, 0xC4, 0xD7, 0xC0, 0xCC, 0xCF, 0xCB, 0xC4, 0x00, 0xCE, 0xC5, 0x00, 0xD2, 0xCF, 0xCB, 0xC8, 0xD3, 0x00, 0xD2, 0xC2, 0xD1, 0xC4, 0xC4, 0xCD, 0x00, 0xD2, 0xC2, 0xD1, 0xCE, 0xCB, 0xCB, 0xC8, 0xCD, 0xC6, 0x00, 0xD4, 0xD2, 0xC8, 0xCD, 0xC6, 0x00, 0xD3, 0xC7, 0xC4, 0x00, 0xC0, 0xD1, 0xC3, 0xD4, 0xC8, 0xCD, 0xCE, 0x00, 0xD4, 0xCD, 0xCE, 0x00, 0xC6, 0xC0, 0xCC, 0xC4, 0x00, 0xC4, 0xCD, 0xC6, 0xC8, 0xCD, 0xC4, 0xDD, 0x00, 0xD3, 0xC7, 0xC4, 0x00, 0xD2, 0xC2, 0xD1, 0xC4, 0xC4, 0xCD, 0x00, 0xC8, 0xD2, 0x00, 0xD2, 0xCF, 0xCB, 0xC8, 0xD3, 0x00, 0xC8, 0xCD, 0xD3, 0xCE, 0x00, 0xC0, 0x00, 0xD2, 0xC8, 0xD7, 0xD3, 0xC4, 0xC4, 0xCD, 0x00, 0xCF, 0xC8, 0xD7, 0xC4, 0xCB, 0x00, 0xC7, 0xC8, 0xC6, 0xC7, 0x00, 0xC5, 0xC8, 0xD7, 0xC4, 0xC3, 0x00, 0xD2, 0xC4, 0xC2, 0xD3, 0xC8, 0xCE, 0xCD, 0x00, 0xC0, 0xD3, 0x00, 0xD3, 0xC7, 0xC4, 0x00, 0xD3, 0xCE, 0xCF, 0xDC, 0x00, 0xD3, 0xC7, 0xC4, 0xCD, 0x00, 0xC0, 0x00, 0xD2, 0xC2, 0xD1, 0xCE, 0xCB, 0xCB, 0xC8, 0xCD, 0xC6, 0x00, 0xD2, 0xC4, 0xC2, 0xD3, 0xC8, 0xCE, 0xCD, 0x00, 0xC8, 0xCD, 0x00, 0xD3, 0xC7, 0xC4, 0x00, 0xCC, 0xC8, 0xC3, 0xC3, 0xCB, 0xC4, 0xDC, 0x00, 0xC0, 0xCD, 0xC3, 0x00, 0xC5, 0xC8, 0xCD, 0xC0, 0xCB, 0xCB, 0xD8, 0x00, 0xC0, 0x00, 0xD2, 0xC8, 0xD7, 0xD3, 0xC4, 0xC4, 0xCD, 0x00, 0xCF, 0xC8, 0xD7, 0xC4, 0xCB, 0x00, 0xC7, 0xC8, 0xC6, 0xC7, 0x00, 0xC5, 0xC8, 0xD7, 0xC4, 0xC3, 0x00, 0xD2, 0xC4, 0xC2, 0xD3, 0xC8, 0xCE, 0xCD, 0x00, 0xC0, 0xD3, 0x00, 0xD3, 0xC7, 0xC4, 0x00, 0xC1, 0xCE, 0xD3, 0xD3, 0xCE, 0xCC, 0xDB, 0x00, 0x00, 0x00, };
const uint8_t bottomText[32] PROGMEM = {0xD3, 0xC7, 0xC8, 0xD2, 0x00, 0xC8, 0xD2, 0x00, 0xD3, 0xC7, 0xC4, 0x00, 0xC5, 0xC8, 0xD7, 0xC4, 0xC3, 0x00, 0xC1, 0xCE, 0xD3, 0xD3, 0xCE, 0xCC, 0x00, 0xD2, 0xC4, 0xC2, 0xD3, 0xC8, 0xCE, 0xCD, };


// Tilemap defs - these are the graphical data for use in the tilemap background. 
// They are arranged in horizontal slices of 256 bytes each as this is fast to render to the display
// These particular defs are from the Space Invaders game
const uint8_t tileData[8 * 256] __attribute__((aligned(256))) PROGMEM = {
	0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFC, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x00, 0x00, 0xF8, 0x0F, 0xF9, 0xFF, 0x1E, 0x73, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xDF, 0x00, 0x00, 0xFC, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0x00, 0x3E, 0x7C, 0x1F, 0xF8, 0x1F, 0x33, 0x99, 0xC0, 0x00, 0xF8, 0x07, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x1F, 0xF0, 0x00, 0x07, 0xFF, 0xCC, 0x0C, 0xF8, 0x07, 0x00, 0x0F, 0x80, 0x00, 0xE0, 0x00, 0x3F, 0xF9, 0x24, 0x92, 0x49, 0x24, 0x92, 0x49, 0x27, 0xFF, 0x00, 0x0E, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0xF8, 0x00, 0x40, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF, 0xFF, 0xFF, 0xFD, 0xFF, 0xFF, 0xFF, 0xF0, 0x30, 0xF8, 0x78, 0xF0, 0xFC, 0xFC, 0x7C, 0xCC, 0xFC, 0x0C, 0xCC, 0xC0, 0xC6, 0xCC, 0x78, 0xF8, 0x78, 0xF8, 0x7C, 0xFC, 0xCC, 0xCC, 0xC6, 0xCC, 0xCC, 0xFC, 0x30, 0x00, 0x00, 0x30, 0x00, 0xC0, 0xCC, 0xCC, 0xCC, 0xCC, 0xC0, 0xC0, 0xCC, 0xCC, 0x30, 0xCC, 0xCC, 0xC0, 0xC6, 0xDC, 0xCC, 0xC0, 0xCC, 0xCC, 0x0C, 0x30, 0xCC, 0x78, 0xFE, 0x78, 0x30, 0xC0, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 
	0x00, 0x0F, 0xFF, 0xFF, 0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0xFF, 0xFF, 0xF8, 0x0F, 0xFF, 0xCF, 0xFE, 0x7F, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x80, 0x01, 0xFC, 0x1F, 0xF8, 0xFE, 0x0E, 0x63, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xDF, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0x00, 0x3E, 0x7F, 0xFF, 0xF8, 0x1F, 0x33, 0x99, 0xCC, 0x0C, 0xF8, 0x07, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x0F, 0xF8, 0x00, 0x0F, 0x87, 0xC0, 0x00, 0xFF, 0xFF, 0x00, 0x0E, 0x00, 0x00, 0x60, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0x0E, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0xF8, 0x00, 0x40, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x01, 0xF0, 0x78, 0xFC, 0xFC, 0xF8, 0xFC, 0xFC, 0xFC, 0xCC, 0xFC, 0x0C, 0xCC, 0xC0, 0xC6, 0xCC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xCC, 0xCC, 0xC6, 0xCC, 0xCC, 0xFC, 0x30, 0x00, 0x00, 0x30, 0x00, 0x60, 0xCC, 0xCC, 0xCC, 0xDC, 0xC0, 0xC0, 0xCC, 0xCC, 0x30, 0xCC, 0xCC, 0xC0, 0xC6, 0xCC, 0xCC, 0xC0, 0xD4, 0xCC, 0x0C, 0x30, 0xCC, 0x78, 0xEE, 0xCC, 0x30, 0xC0, 0x00, 0x30, 0x30, 0x00, 0x30, 0x70, 
	0x00, 0x0F, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0xFF, 0xFF, 0xFC, 0x1F, 0xFE, 0x01, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x80, 0x01, 0xFF, 0xFF, 0xFC, 0x7C, 0x43, 0xC7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0x80, 0x7E, 0x78, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF1, 0xE3, 0xFB, 0x37, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x1F, 0xFC, 0x00, 0x1F, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0E, 0x00, 0x00, 0xF8, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0x0F, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0x7F, 0x03, 0xF8, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0xFC, 0x00, 0xE0, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFC, 0xCC, 0xCC, 0xDC, 0xC0, 0xC0, 0xC0, 0xCC, 0x30, 0x0C, 0xD8, 0xC0, 0xEE, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xC0, 0x30, 0xCC, 0xCC, 0xC6, 0xCC, 0xCC, 0x18, 0x30, 0x00, 0x00, 0x30, 0x30, 0x70, 0xCC, 0xFC, 0xFC, 0xF8, 0xFC, 0xC0, 0xFC, 0xCC, 0xFC, 0xFC, 0xC6, 0xFC, 0xC6, 0xCC, 0xFC, 0xC0, 0xF8, 0xC6, 0xFC, 0x30, 0xFC, 0x30, 0xC6, 0xCC, 0x30, 0xFC, 0x00, 0x30, 0x30, 0x30, 0x30, 0x60, 
	0x00, 0x0F, 0xFF, 0xFF, 0xE0, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x30, 0xE1, 0x87, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x00, 0x00, 0xF8, 0x0F, 0xFC, 0x00, 0xE0, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF, 0xE6, 0x67, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF3, 0xF3, 0xFB, 0x37, 0x00, 0x0F, 0xFF, 0xFF, 0xFC, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0E, 0x00, 0x07, 0xE0, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0xF0, 0x01, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0x3E, 0x01, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE3, 0x00, 0xFF, 0xFF, 0xFE, 0xFE, 0x01, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xC0, 0xC0, 0xC0, 0xCC, 0x30, 0x0C, 0xD8, 0xC0, 0xEE, 0xEC, 0xCC, 0xCC, 0xCC, 0xCC, 0xC0, 0x30, 0xCC, 0xCC, 0xC6, 0x78, 0xCC, 0x18, 0x30, 0x00, 0x00, 0x30, 0x30, 0x30, 0xCC, 0xF8, 0x78, 0xF0, 0xFC, 0xC0, 0x78, 0xCC, 0xFC, 0x78, 0xC6, 0xFC, 0xC6, 0xCC, 0x78, 0xC0, 0x6C, 0xC6, 0xF8, 0x30, 0x78, 0x30, 0x82, 0xCC, 0x30, 0xFC, 0x00, 0x30, 0x30, 0x30, 0x30, 0xC0, 
	0x00, 0x0F, 0xFF, 0xFF, 0xE0, 0x7E, 0x80, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x03, 0xF0, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0xE6, 0x67, 0xF8, 0x0F, 0xFC, 0xFC, 0x47, 0xE7, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x00, 0x00, 0xF8, 0x0F, 0xFF, 0x03, 0xF8, 0x1F, 0xF8, 0x70, 0x7F, 0xE7, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x1F, 0xFF, 0xF3, 0xF3, 0xF8, 0x07, 0x00, 0x0F, 0xFF, 0xFF, 0xF8, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0E, 0x00, 0x00, 0xE0, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0x0F, 0xFF, 0xFF, 0xF0, 0x01, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0x3C, 0x00, 0xE0, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC1, 0x00, 0xFF, 0xFF, 0xFE, 0xFF, 0x03, 0xF8, 0x1F, 0xC0, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0xFC, 0xC0, 0xCC, 0xF8, 0xF8, 0xDC, 0xFC, 0x30, 0x0C, 0xF0, 0xC0, 0xFE, 0xEC, 0xCC, 0xCC, 0xCC, 0xCC, 0xE0, 0x30, 0xCC, 0xCC, 0xC6, 0x78, 0x78, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 
	0x00, 0x0F, 0xFF, 0xFF, 0xF0, 0xFE, 0xB3, 0x33, 0x33, 0x33, 0x7D, 0x99, 0x99, 0x99, 0x9B, 0xF0, 0x00, 0x0F, 0xFF, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x80, 0x01, 0xF8, 0x0F, 0xF9, 0xFE, 0x0E, 0x73, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xCF, 0xFE, 0x7F, 0xF0, 0x20, 0x3F, 0xC3, 0x00, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0x00, 0x0F, 0xFF, 0xFF, 0x82, 0x1F, 0xF8, 0x0F, 0xF8, 0x00, 0x0F, 0xFF, 0xF3, 0xF3, 0xF8, 0x07, 0x00, 0x0F, 0xFF, 0xFF, 0xE0, 0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0E, 0x00, 0x00, 0x78, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0x03, 0xFF, 0xFF, 0xF0, 0x01, 0xFF, 0xF9, 0x24, 0x92, 0x49, 0x24, 0x92, 0x49, 0x27, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0x38, 0x00, 0x40, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC1, 0x00, 0xFC, 0x00, 0x01, 0xFF, 0xCF, 0xFE, 0x7F, 0xDF, 0xFF, 0xFF, 0xFD, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC, 0xF8, 0xC0, 0xCC, 0xF8, 0xF8, 0xDC, 0xFC, 0x30, 0x0C, 0xF0, 0xC0, 0xD6, 0xFC, 0xCC, 0xCC, 0xCC, 0xFC, 0x70, 0x30, 0xCC, 0xCC, 0xD6, 0x30, 0x78, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x0F, 0xFF, 0xFF, 0xF9, 0xFE, 0xB3, 0x33, 0x33, 0x33, 0x7D, 0x99, 0x99, 0x99, 0x9B, 0xF0, 0x00, 0x0F, 0x1F, 0xFF, 0x81, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x80, 0x01, 0xFF, 0xFF, 0xF9, 0xFF, 0x1E, 0x73, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x80, 0x01, 0xFC, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x70, 0x7F, 0xC3, 0x00, 0x0F, 0x1F, 0xFF, 0x80, 0x7E, 0x7C, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0x37, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x0F, 0xF0, 0x00, 0x07, 0xFF, 0xF3, 0xF3, 0xFB, 0x37, 0x00, 0x0F, 0xFF, 0xFF, 0xE0, 0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0E, 0x00, 0x00, 0xFC, 0x01, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0xFC, 0x00, 0x01, 0xF0, 0x01, 0xFF, 0xF9, 0x24, 0x92, 0x49, 0x24, 0x92, 0x49, 0x27, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0x38, 0x00, 0x40, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC1, 0x00, 0x03, 0xFE, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xD3, 0x33, 0x33, 0x3D, 0x33, 0x33, 0x33, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xCC, 0xC0, 0xCC, 0xC0, 0xC0, 0xCC, 0xCC, 0x30, 0x0C, 0xD8, 0xC0, 0xD6, 0xFC, 0xCC, 0xFC, 0xCC, 0xF8, 0x38, 0x30, 0xCC, 0xCC, 0xD6, 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFE, 0x80, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x03, 0xF0, 0x00, 0x0E, 0x0F, 0xFF, 0xC3, 0xC3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xDF, 0x00, 0x00, 0xFC, 0x1F, 0xF9, 0x83, 0x18, 0x13, 0xFF, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xDF, 0x00, 0x00, 0xF8, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0x00, 0x0F, 0xFF, 0xFF, 0x00, 0x3E, 0x78, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0x37, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x07, 0xFF, 0xF1, 0xE3, 0xFB, 0x37, 0x00, 0x0F, 0xFF, 0xFF, 0xF8, 0x00, 0xFF, 0xF9, 0x24, 0x92, 0x49, 0x24, 0x92, 0x49, 0x27, 0xFF, 0x00, 0x0E, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0xF0, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFE, 0x78, 0x00, 0x40, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE3, 0x00, 0x0F, 0xFF, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xD3, 0x33, 0x33, 0x3D, 0x33, 0x33, 0x33, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xCC, 0xC0, 0xCC, 0xC0, 0xC0, 0xCC, 0xCC, 0x30, 0x0C, 0xD8, 0xC0, 0xC6, 0xDC, 0xCC, 0xF8, 0xCC, 0xD8, 0x1C, 0x30, 0xCC, 0xCC, 0xFE, 0x78, 0x30, 0x60, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

// Sprite defs - each sprite is 16x16 pixels arranged as 2 bytes horizontally, by 16 vertically. So 32 bytes per sprite.
// These can be assigned to a sprite using the setSpriteDef() function.
// These particular sprite defs are the Miner Willy sprite frames from the Manic Miner game
const uint8_t spriteDef[(16*(16*2))] __attribute__((aligned(32))) PROGMEM = {

	0x01,0x80,0x0F,0x80,0x1F,0x00,0x0D,0x00,0x0F,0x80,0x0F,0x00,0x06,0x00,0x0F,0x00,0x1F,0x80,0x1F,0x80,0x3D,0xC0,0x3E,0xC0,0x0F,0x00,0x1D,0x80,0x1B,0x80,0x1D,0xC0,
	0x01,0x80,0x0F,0x80,0x1F,0x00,0x0D,0x00,0x0F,0x80,0x0F,0x00,0x06,0x00,0x0F,0x00,0x1B,0x80,0x1B,0x80,0x1B,0x80,0x1D,0x80,0x0F,0x00,0x06,0x00,0x06,0x00,0x07,0x00,
	0x01,0x80,0x0F,0x80,0x1F,0x00,0x0D,0x00,0x0F,0x80,0x0F,0x00,0x06,0x00,0x0F,0x00,0x1F,0x80,0x1F,0x80,0x3D,0xC0,0x3E,0xC0,0x0F,0x00,0x1D,0x80,0x1B,0x80,0x1D,0xC0,
	0x01,0x80,0x0F,0x80,0x1F,0x00,0x0D,0x00,0x0F,0x80,0x0F,0x00,0x06,0x00,0x0F,0x00,0x1F,0x80,0x3F,0xC0,0x7F,0xE0,0x6F,0x60,0x0F,0x80,0x1D,0xA0,0x30,0xE0,0x38,0x40,

	0x06,0x00,0x07,0xC0,0x03,0xE0,0x02,0xC0,0x07,0xC0,0x03,0xC0,0x01,0x80,0x03,0xC0,0x07,0xE0,0x0F,0xF0,0x1F,0xF8,0x1B,0xD8,0x07,0xC0,0x16,0xE0,0x1C,0x30,0x08,0x70,
	0x06,0x00,0x07,0xC0,0x03,0xE0,0x02,0xC0,0x07,0xC0,0x03,0xC0,0x01,0x80,0x03,0xC0,0x07,0xE0,0x07,0xE0,0x0E,0xF0,0x0D,0xF0,0x03,0xC0,0x06,0xE0,0x07,0x60,0x0E,0xE0,
	0x06,0x00,0x07,0xC0,0x03,0xE0,0x02,0xC0,0x07,0xC0,0x03,0xC0,0x01,0x80,0x03,0xC0,0x07,0xE0,0x07,0x60,0x07,0x60,0x06,0xE0,0x03,0xC0,0x01,0x80,0x01,0x80,0x03,0x80,
	0x06,0x00,0x07,0xC0,0x03,0xE0,0x02,0xC0,0x07,0xC0,0x03,0xC0,0x01,0x80,0x03,0xC0,0x07,0xE0,0x07,0xE0,0x0E,0xF0,0x0D,0xF0,0x03,0xC0,0x06,0xE0,0x07,0x60,0x0E,0xE0,
	
};

#endif
