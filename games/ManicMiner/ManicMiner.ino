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

/**
 * 
 * Original game by Matthew Smith for ZX Spectrum 48K in 1983. 
 * 
 * Thanks to ceptimus on YouTube for information about the correct timings for the sync pulses!
 * 
 */

#include "defs.h"
#include "types.h"
#include "gameAssets.h"
#include "vars.h"
#include "func_defs.h"
#include "audio.h"
#include "sprite.h"
#include "debug.h"
#include "tileMap.h"
#include "input.h"
#include "funcs.h"
#include "game.h"

// Called at the start of each scan line's sync pulse, even when not drawing
ISR(TIMER1_OVF_vect){ 
	if(++scanline==312){
		OCR1A=948;  // scan lines 0 - 7 have wide 59.3us sync pulses
		scanline=0;
	}else if(scanline==8){
		OCR1A = 74; // scan lines 8 - 311 have short 4.7us sync pulses
	}else if(scanline==TOP_EDGE){
    // Start interrupt to output pixels. Enabling interrupt causes it to fire immediately, 
    // so we use the first trigger to set up variables for rendering to display
    #if USE_MASK
    OCR1B = LEFT_EDGE+(hScroll*2); // Used for "hardware" horizontal scrolling if enabled (not used in Manic Miner)
    #endif
		TIMSK1 |= _BV(OCIE1B);
	}
}

// Only called for initial frame setup scanline (not drawn), then active scanlines
ISR(TIMER1_COMPB_vect) {
	static int8_t slice;
  static uint8_t screenRamRow;
	tcnt = TCNT1; // capture timer to allow jitter correction
	
	if(scanline==TOP_EDGE) { // on stored-up 'false trigger' scanline, initialize the pointers etc
		slice = (vScroll&0x07);
    if(altFontLine==0){
     tileDataIX=altFontIX;
    }
    screenRamRow=(tileRowStart-1)+((vScroll>>3)%(CHARACTER_ROWS-tileRowStart));
		pScreenRam = screenRam+(screenRamRow*BYTES_PER_BUFFER_LINE); // point to first character (top left) in screenRam
		fontSlice = tileData[tileDataIX]+(slice*256); // point to slice before first (top) slice of font pixels (top pixel of each 10 is just RVS cap)
    displayLine=(scanline-TOP_EDGE)+1;
    collisionBits=0;  // Reset collisionBit flag register
	} else {
		renderScanline(pScreenRam, fontSlice, tcnt, minTCNT);
    displayLine=(scanline-TOP_EDGE)+1;
    if(displayLine==lineCompare-8){
      slice = -1;
      fontSlice=tileData[tileDataIX]-256;
      screenRamRow=0;
      pScreenRam=screenRam+(screenRamRow*BYTES_PER_BUFFER_LINE);
      #if USE_MASK
      OCR1B = LEFT_EDGE-3;  // If horizontal scrolling in use, ensure scroll position is reset when lineCompare is triggered!
      hScrollSMask=0x7f>>(7-0);
      hScrollEMask=0xfe<<0;
      #endif
    }
    if(displayLine==altFontLine){
      if(altFontLine<255){
        tileDataIX=altFontIX;
      }
    }else if(displayLine==restoreFontLine){
      tileDataIX=0;
    }
		if (tcnt < minTCNT) 
		  minTCNT = tcnt;
		if (scanline == BOTTOM_EDGE) {
			TIMSK1 &= ~_BV(OCIE1B); // we don't want any more COMPB interrupts this frame
      currentFrame++;
		} else if(++slice==PIXELS_PER_CHARACTER){
			slice = 0;
			fontSlice=tileData[tileDataIX];
      screenRamRow++;
      if(screenRamRow==(CHARACTER_ROWS)){
        screenRamRow=tileRowStart;
        pScreenRam=screenRam+(screenRamRow*BYTES_PER_BUFFER_LINE);
      }else{
			  pScreenRam+=BYTES_PER_BUFFER_LINE;
      }
		} else {
			fontSlice += 256;
		}
   
	}
}

void setup() {
  initHardware();
  setState(title);
}

void loop() {
  gameLoop();  
}
