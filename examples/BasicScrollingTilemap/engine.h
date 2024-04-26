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

#ifndef __ENGINE
#define __ENGINE

#include "engineDefs.h"
//#include "avr/io.h"

typedef bool boolean;

typedef struct ProcessedSprite{
    uint8_t yLine;        // Scanline at top of sprite
    uint8_t xByte;        // Byte position in line buffer
    uint8_t sData[3*16]; // Graphical data, pre-shifted ready to display on scanlines
} ProcessedSprite;

typedef struct SpriteData{
    uint8_t y;
    uint8_t x;
    uint8_t defIX;
} SpriteData;

typedef struct KeyState{
    uint8_t current;
    uint8_t last;
    uint8_t changed;
} KeyState;

extern "C" { void processSprite(ProcessedSprite *dest, const uint8_t *spriteDefRef, const SpriteData *spriteData); }
extern "C" { void renderScanline(volatile uint8_t *pScreenRam, volatile const uint8_t *fontSlice); }
extern "C" { void loadHi(); }
extern "C" { void storeHi(); }

//#include <avr/iom328p.h>
#include "gameAssets.h"

volatile uint16_t scanline = 0; // counts 0 - 311 (312 scan lines per frame)
volatile uint16_t minTCNT = 0xFFFF;
volatile const uint8_t *fontSlice;
volatile uint8_t currentFrame=0;  // Increments every frame, automatically wraps back to 0
volatile uint8_t lastFrame=0;
volatile uint8_t displayLine=0;
volatile uint8_t collisionBits=0; // This is set to non-zero if sprite at index 8 has collided with another sprite during the last frame (pixel based collision detection)
volatile uint8_t *pScreenRam;
volatile uint8_t spriteLine[BYTES_PER_RASTER+2]__attribute__((aligned(64))) ={0};
volatile uint16_t tcnt;

const uint8_t *spriteDefRef;  // ASM uses this pointer to the const sprite data array - this gives the possibility of bank switching sprites if there are more than 256 (unlikely as that is!)
uint16_t hScroll=0;
uint16_t vScroll=0;
uint8_t tileRowStart=0;
uint16_t topLineCompare=0;     // Set using setTileRowSplit() below
uint16_t bottomLineCompare=600;     // Set using setTileRowSplit() below
uint8_t hScrollSMask=0;
uint8_t hScrollEMask=0;
uint8_t hScrollSMaskReset=0;
uint8_t hScrollEMaskReset=0;
uint8_t hSoffReset=0;
uint8_t ocr1bReset=0;
uint8_t screenRamRowTopReset=0;
uint8_t screenRamRowBottomReset=0;
uint8_t *pScreenRamBottomReset;

KeyState keyState;
uint16_t hiScore=0;
uint16_t waitFrames=0;

// y,x,defIX
SpriteData spriteData[NUM_SPRITES]={0};
ProcessedSprite processedSprite[NUM_SPRITES]={0};
uint8_t screenRam[SCREEN_RAM_SIZE]={0};



// Declarations
void engineSetup();
void setPWMChannel(const uint8_t ix, const uint8_t isOn);
void setPWMChannelFreq(const uint8_t ix,const uint8_t freq);
void setPWMChannelBaseSetting(const uint8_t ix, const uint8_t baseSetting);
inline void waitFrame();
uint8_t readInput();
void processSprites();
void setSpritePos(uint8_t ix,uint8_t x, uint8_t y);
void setSpriteDef(uint8_t ix, uint8_t defIX);
void setTileRowSplit(uint16_t topY, uint16_t bottomY);
void setScroll(uint8_t x, uint8_t y);
void clearTileMap(const uint8_t charIX);
void shiftTiles(int8_t offset,uint8_t fillTile);



// Implementations


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
		TIMSK1 |= _BV(OCIE1B);
	}
}

// Only called for initial frame setup scanline (not drawn), then active scanlines
ISR(TIMER1_COMPB_vect) {
	static int8_t slice;
  static uint8_t screenRamRow;
	static uint8_t hSoff;
	tcnt = TCNT1-hSoff; // capture timer to allow jitter correction - need to also offset the hScroll position, as it's added to the timer counter!

	if(scanline==TOP_EDGE) { // on stored-up 'false trigger' scanline, initialize the pointers etc
    displayLine=0;
    
    if(topLineCompare==displayLine){

      #if USE_MASK
        hScrollSMask=hScrollSMaskReset;
        hScrollEMask=hScrollEMaskReset;
        OCR1B = ocr1bReset;
        hSoff=hSoffReset;
      #else
        hSoff=0;
      #endif
      slice=(vScroll&0x07);
      screenRamRow=screenRamRowTopReset;

    }else{

      #if USE_MASK
        OCR1B = LEFT_EDGE;  // If horizontal scrolling in use, ensure scroll position is reset when lineCompare is triggered!
        hScrollSMask=255;//HSCROLL_S_RESET;
        hScrollEMask=255; //HSCROLL_E_RESET;
      #else
        hSoff=0;
      #endif
      slice=0;
      hSoff=0;
      screenRamRow=0;
    }

    collisionBits=0;  // Reset collisionBit flag register
		pScreenRam = screenRam+(screenRamRow*BYTES_PER_BUFFER_LINE); // point to first character (top left) in screenRam
		fontSlice = tileData+(slice*256); // point to slice before first (top) slice of font pixels (top pixel of each 10 is just RVS cap)
    displayLine=1;

	} else {

		renderScanline(pScreenRam, fontSlice);

    if(displayLine==bottomLineCompare){ // Note that displayLine is incremented in the ASM routine...

      slice = -1;
      fontSlice=tileData-256;
      screenRamRow=screenRamRowBottomReset;
      pScreenRam=pScreenRamBottomReset;
      
      #if USE_MASK
        OCR1B = LEFT_EDGE;  // If horizontal scrolling in use, ensure scroll position is reset when lineCompare is triggered!
        hSoff=0;
        hScrollSMask=255;//HSCROLL_S_RESET;
        hScrollEMask=255;//HSCROLL_E_RESET;
      #endif

    }else if(topLineCompare==displayLine){
      #if USE_MASK
        hScrollSMask=hScrollSMaskReset;
        hScrollEMask=hScrollEMaskReset;
        OCR1B = ocr1bReset;
        //OCR1B = LEFT_EDGE+((7-(hScroll&0x07))*2);
        hSoff=hSoffReset;
        minTCNT+=hSoff;
      #else
        hSoff=0;
      #endif
      slice=(vScroll&0x07);
      screenRamRow=screenRamRowTopReset;
      //screenRamRow=tileRowStart+((vScroll>>3)%(CHARACTER_ROWS-tileRowStart));
      pScreenRam = screenRam+(screenRamRow*BYTES_PER_BUFFER_LINE); // point to first character (top left) in screenRam
		  fontSlice = tileData+(slice*256); // point to slice before first (top) slice of font pixels (top pixel of each 10 is just RVS cap)
    }

    if(displayLine==BOTTOM_DISPLAY_LINE){
      
        TIMSK1 &= ~_BV(OCIE1B); // we don't want any more COMPB interrupts this frame
        currentFrame++;

    }else if(++slice==PIXELS_PER_CHARACTER){

        slice = 0;
        fontSlice=tileData+0;
        if(++screenRamRow==CHARACTER_ROWS){
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


/**
 * @brief Initialise the engine. Sets up timers etc. After this, the video interrupt will be started and run continuously
 * 
 */
void engineSetup(){
  loadHi(); // Load from EEPROM before we start the video output, as we steal some of the EEPROM registers for use in the rendering code - we can still store to EEPROM, but need to ensure there is no clash :)  
  spriteDefRef=spriteDef+0;

  minTCNT = 0xFFFF;
  // Set timer 2 for PWM output on D11 (PORTB bit 3)
  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS21) | _BV(CS20);

  // Set timer 0 for output on D6
  TCCR0A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM01) | _BV(WGM00);
  TCCR0B = _BV(WGM22) | _BV(CS01) | _BV(CS00);

#if USE_CONTROLLER

  // Configure pins for clocking data from NES controller
  PORTB=((1<<NES_DATA)|(1<<NES_CLOCK));
  DDRB=((1<<NES_LATCH)|(1<<NES_CLOCK));

#else

  // Configure input pins for buttons
  PORTC=0b111111; // pullups enabled
  PORTD|=(1<<2);     // Option button on D2
  PORTB|=(1<<0);     // Start button on B0

#endif

  // configure USART as master SPI mode 0, MSB first, 8MHz
  UCSR0A = _BV(U2X0); // double speed
  UCSR0B = _BV(TXEN0);
  UCSR0C = _BV(UMSEL01) | _BV(UMSEL00);
  UBRR0L = UBRR0H = 0x00; // fastest possible baud
  
  // output pin for sync pulses - low 4.7 us pulses at start of visible scan lines;  longer low pulses for vertical blank
  DDRB|=(1<<1);

  // configure timer/counter 1 to output scanline sync pulses on Pin9 (OC1A)
  // use mode 7 (fast PWM 10-bit count to TOP=1023) at 16MHz fclk - one cycle per 64us scanline
  cli(); // not necessary
  TCCR1A =  _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11) | _BV(WGM10); // set OC1A output on compare match, (mode 3 so far)
  TCCR1B = _BV(WGM12) | _BV(CS10); // now mode 7 at clk/1 (16MHz)
  OCR1A = 948; // 59.3us wide sync pulse for first 8 scan lines
  OCR1B = LEFT_EDGE;
  TIMSK1 = _BV(TOIE1); // _BV(OCIE1A);
  TCNT1 = 0x0000;
  sei(); // necessary
  TIMSK0 &= ~_BV(TOIE0); // disable timer0 - stops millis() working but necessary to stop timer 0 interrupts spoiling display timing

  readInput();
}

/**
 * @brief mute/unmute a PWM channel
 * 
 * @param ix The channel index, either 0 or 1
 * @param isOn 0=off, 1=on
 * @return * Basic 
 */
void setPWMChannel(const uint8_t ix, const uint8_t isOn){
  if(ix==0){
    if(isOn){
      UNMUTE_PWM_0;
    }else{
      MUTE_PWM_0;
    }
  }else if(ix==1){
    if(isOn){
      UNMUTE_PWM_1;
    }else{
      MUTE_PWM_1;
    }
  }
}

/**
 * @brief Set the frequency of one channel relative to its base frequency
 * 
 * @param ix The channel index, either 0 or 1
 * @param f The frequency value (0-255)
 * @param baseFreq The bits to set the clock divider - for example _BV(WGM22) | _BV(CS01) | _BV(CS00) or _BV(WGM22) | _BV(CS22)
 */
void setPWMChannelFreq(const uint8_t ix,const uint8_t freq){
  if(ix==0){
    OCR0A=freq;
  }else{
    OCR2A=freq;
  }
  if(freq==0){
    setPWMChannel(ix,0);
  }else{
    setPWMChannel(ix,1);
  }
}

/**
 * @brief Set the base timing for a channel
 * 
 * @param ix The channel index
 * @param baseSetting The bits to set the clock divider - for example _BV(CS01) | _BV(CS00) or| _BV(CS22)
 */
void setPWMChannelBaseSetting(const uint8_t ix, const uint8_t baseSetting){
  if(ix==0){
    TCCR0B=_BV(WGM22) | baseSetting;
  }else{
    TCCR2B=_BV(WGM22) | baseSetting;
  }
  
}

// Misc functions

/**
 * @brief Wait for the current frame to finish drawing. Game loop should use this to ensure the game is synchronised to the display update of 50fps
 * 
 */
inline void waitFrame(){
    waitFrames=0;
    while(currentFrame==lastFrame){
      ++waitFrames;
    }
    lastFrame=currentFrame;
}

/**
 * @brief Note that a LOW bit = button pressed
 * Order of bits from LSb->MSb = "B","A","Down","Up","Right","Left","Start","Option" (Start not yet connected)
 * First 6 inputs are on PORTC, Option on PORTD2, Start on PORTD0
 */
uint8_t readInput(){
  keyState.last=keyState.current;

#if USE_CONTROLLER

  PORTB|=(1<<NES_LATCH);
  PORTB&=~(1<<NES_LATCH);
  for(uint8_t n=0;n<8;n++){
    PORTB&=~(1<<NES_CLOCK);
    keyState.current<<=1;
    uint8_t i=PINB>>NES_DATA;
    keyState.current+=( i & 0x01 );
    PORTB|=(1<<NES_CLOCK);
  }

#else

  keyState.current=PINC&0x3f;
  keyState.current|=((PINB&(1<<0))<<6); // Option button on PORTB
  keyState.current|=((PIND&(1<<2))<<5); // Start button on PORTD

#endif

  keyState.changed=(keyState.current^keyState.last);
  return keyState.changed;
}



// Sprite funcs

/**
 * @brief After changing sprite positions or definitions ready for next frame, this must be called to create the buffers used by the renderer to draw sprite data to the correct positions
 * It should be called exactly once per frame, after all sprites have been positioned, and their graphical definitions set.
 */
void processSprites(){
    for(uint8_t n=0;n<NUM_SPRITES;n++){
        processSprite(&processedSprite[n], spriteDef, &spriteData[n]);  
    }
}

/**
 * @brief Set the Sprite position
 * 
 * @param ix Index of sprite, from 0 to NUM_SPRITES-1
 * @param x The pixel position in X axis
 * @param y The pixel position in Y axis (top=0)
 */
void setSpritePos(uint8_t ix,uint8_t x, uint8_t y){
  spriteData[ix].y=y;
  spriteData[ix].x=x;
}

/**
 * @brief Set the Sprite definition (graphical data for a sprite)
 * 
 * @param ix Index of sprite
 * @param defIX The graphical definition data to use
 */
void setSpriteDef(uint8_t ix, uint8_t defIX){
    spriteData[ix].defIX=defIX;
}


// Tilemap funcs 

/**
 * @brief Set the scanlines at which to split the display. Once the renderer reaches the topY line it will begin drawing from the top of screenRAM+(static RAM lines).
 * Once it reaches bottomY it will resume drawing from top of screenRAM. So think of the static top and bottom sections to be in the top of screen RAM, and the centre
 * scrolling section to be after that initial static section. This allows the centre of the screen to scroll and X lines at the top and bottom to remain static for
 * score, status etc if needed. If no static sections are required, then set topY to 0 and bottomY to a value higher than 255.
 * 
 * @param topY The scanline at which to allow scrolling to start
 * @param bottomY The scanline at which to stop scrolling section and revert to drawing from top of screen RAM
 */
void setTileRowSplit(uint16_t topY,uint16_t bottomY){
    topLineCompare=topY;
    bottomLineCompare=bottomY;
    screenRamRowBottomReset=(topLineCompare/PIXELS_PER_CHARACTER);
    tileRowStart=CHARACTER_ROWS-(bottomLineCompare/PIXELS_PER_CHARACTER);
    tileRowStart+=screenRamRowBottomReset;

    pScreenRamBottomReset=screenRam+(screenRamRowBottomReset*BYTES_PER_BUFFER_LINE);

}

/**
 * @brief Set the scroll position in X and Y axis for the tilemap. This does not affect sprites
 * 
 * @param x Scroll position in x axis (pixels - 0-255)
 * @param y Scroll position in y axie (pixels - 0-255)
 */
void setScroll(uint8_t x, uint8_t y){
    vScroll=y;
    #if USE_MASK==1
    hScroll=x;
    uint8_t a=(uint8_t)(hScroll&0x07);
    hScrollSMaskReset=0x7f>>a;
    hScrollEMaskReset=0xfe<<(7-a);
    hSoffReset=((7-a)*2);
    ocr1bReset=LEFT_EDGE+hSoffReset;
    #else
    hScrollSMask=0;
    hScrollEMask=0;
    ocr1bReset=LEFT_EDGE
    #endif
    screenRamRowTopReset=tileRowStart+((vScroll>>3)%(CHARACTER_ROWS-tileRowStart));
}

/**
 * @brief Set all tiles in the background tilemap to a character index
 * 
 * @param charIX The character index to use
 */
void clearTileMap(const uint8_t charIX){
  uint8_t *p=screenRam;
  for(uint16_t n=0;n<BYTES_PER_BUFFER_LINE*CHARACTER_ROWS;n++){
    *p++=charIX;
  }
}

/**
 * @brief Move all tiles in the tilemap left/right by x character places. Useful for horizontal scrolling
 * 
 * @param offset amount of tiles to shift, from -32 to 32
 * @param fillTile The index of the tile to fill the blank space to the left/right after shifting tiles
 */
void shiftTiles(int8_t offset,uint8_t fillTile){
  if(offset<0){
    uint8_t *p=screenRam+(tileRowStart*BYTES_PER_BUFFER_LINE);
    const uint8_t off=-offset;
    for(uint8_t row=tileRowStart;row<CHARACTER_ROWS;row++){
      uint8_t shiftNum=BYTES_PER_BUFFER_LINE-off;
      while(shiftNum){
        *p=*(p+off);
        ++p;
        --shiftNum;
      }
      uint8_t blankNum=off;
      while(blankNum){
        *p++=fillTile;
        --blankNum;
      }
    }
  }else{
    uint8_t *p=screenRam+(SCREEN_RAM_SIZE-1);
    const uint8_t off=offset;
    for(uint8_t row=tileRowStart;row<CHARACTER_ROWS;row++){
      uint8_t shiftNum=BYTES_PER_BUFFER_LINE-off;
      while(shiftNum){
        *p=*(p-off);
        --p;
        --shiftNum;
      }
      uint8_t blankNum=off;
      while(blankNum){
        *p--=fillTile;
        --blankNum;
      }
    }
  }
}

#endif
