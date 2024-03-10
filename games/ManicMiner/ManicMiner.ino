/*
 * From UART 150R       D1
 * From Suppress 0R     D4
 * From Sync 1.2K       B1
 * FROM PWMA 470R       B3
 * From PWMB 470R       D6
 * B                    C0
 * A                    C1
 * Down                 C2
 * Up                   C3
 * Right                C4
 * Left                 C5
 * Start                B0
 * Option               D2
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

ISR(TIMER1_OVF_vect) { // TIMER1_OVF vector occurs at the start of each scan line's sync pulse
	if (++scanline == 312) {
		OCR1A = 948; // scan lines 0 - 7 have wide 59.3us sync pulses
		scanline = 0;
	} else if (scanline == 8) {
		OCR1A = 74; // swap to short 4.7us sync pulses for scan lines 8 - 311
		// asm("SBI 3, 5"); // toggle LED_BUILTIN
		// enabling the interrupt generates an immediate 'stored up' interrupt
		// so enable it one scan line early, test and return within interrupt handler to ignore 1st one
	}	else if (scanline == TOP_EDGE) { // scan line 51 is first 'text safe' scan line - will already have been incremented to 52 here
		TIMSK1 |= _BV(OCIE1B);
	}
}

ISR(TIMER1_COMPB_vect) { // occurs at start of 'text safe' area of scan lines 51 - 280
	static int8_t slice;
  static uint8_t screenRamRow;
	tcnt = TCNT1; // capture timer to allow jitter correction
	
	if(scanline==TOP_EDGE) { // on stored-up 'false trigger' scanline, initialize the pointers etc
    #if USE_MASK
    OCR1B = LEFT_EDGE+(hScroll*2);
    #endif
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
      OCR1B = LEFT_EDGE-3;
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
