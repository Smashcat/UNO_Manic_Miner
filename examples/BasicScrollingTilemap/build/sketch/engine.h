#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\examples\\BasicScrollingTilemap\\engine.h"
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
extern "C" { void renderScanline(volatile uint8_t *pScreenRam, volatile const uint8_t *fontSlice, volatile uint16_t tcnt, volatile uint16_t minTCNT); }
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

const uint8_t *spriteDefRef;  // ASM uses this pointer to the const sprite data array - this gives the possibility of bank switching sprites if there are more than 256 (unlikely as that is!)
uint16_t hScroll=0;
uint16_t vScroll=0;
uint8_t tileRowStart=0;
uint16_t lineCompare=600;     // Set using setTileRowSplit() below
uint8_t hScrollSMask=0x0;
uint8_t hScrollEMask=0x0;
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
void setTileRowSplit(uint16_t x);
void setScroll(uint8_t x, uint8_t y);
void clearTileMap(const uint8_t charIX);

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
    #if USE_MASK
    OCR1B = LEFT_EDGE+(hScroll*2); // Used for "hardware" horizontal scrolling if enabled (not used in this game)
    #endif
		TIMSK1 |= _BV(OCIE1B);
	}
}

// Only called for initial frame setup scanline (not drawn), then active scanlines
ISR(TIMER1_COMPB_vect) {
	uint16_t tcnt = TCNT1; // capture timer to allow jitter correction
	static int8_t slice;
  static uint8_t screenRamRow;
	
	if(scanline==TOP_EDGE) { // on stored-up 'false trigger' scanline, initialize the pointers etc
		slice=(vScroll&0x07);
    screenRamRow=tileRowStart+((vScroll>>3)%(CHARACTER_ROWS-tileRowStart));
		pScreenRam = screenRam+(screenRamRow*BYTES_PER_BUFFER_LINE); // point to first character (top left) in screenRam
		fontSlice = tileData+(slice*256); // point to slice before first (top) slice of font pixels (top pixel of each 10 is just RVS cap)
    displayLine=(scanline-TOP_EDGE)+1;
    collisionBits=0;  // Reset collisionBit flag register
	} else {
		renderScanline(pScreenRam, fontSlice, tcnt, minTCNT);
    if (tcnt < minTCNT) 
      minTCNT = tcnt;
    displayLine=(scanline-TOP_EDGE)+1;
    if(displayLine==lineCompare){
        slice = -1;
        fontSlice=tileData-256;
        screenRamRow=0;
        pScreenRam=screenRam+(screenRamRow*BYTES_PER_BUFFER_LINE);
        #if USE_MASK
        OCR1B = LEFT_EDGE-3;  // If horizontal scrolling in use, ensure scroll position is reset when lineCompare is triggered!
        hScrollSMask=0x7f>>(7-0);
        hScrollEMask=0xfe<<0;
        #endif
    }
    if (scanline == BOTTOM_EDGE) {
        TIMSK1 &= ~_BV(OCIE1B); // we don't want any more COMPB interrupts this frame
        currentFrame++;
    } else if(++slice==PIXELS_PER_CHARACTER){
        slice = 0;
        fontSlice=tileData+0;
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
 * @brief Set the scanline at which to split the display. Once the renderer reaches this line it will begin to draw from the top row of tiles in the screenRam. 
 *        Useful for games that scroll and require a static status bar.
 * 
 * @param x The scanline at which to split - if over 255, the split will not occur
 */
void setTileRowSplit(uint16_t x){
    lineCompare=x;
    tileRowStart=CHARACTER_ROWS-(lineCompare/PIXELS_PER_CHARACTER);
}

/**
 * @brief Set the scroll position in X and Y axis for the tilemap. This does not affect sprites
 * 
 * @param x Scroll position in x axis (pixels)
 * @param y Scroll position in y axie (pixels)
 */
void setScroll(uint8_t x, uint8_t y){
    hScroll=x;
    vScroll=y;
    #if USE_MASK==1
    uint8_t a=(uint8_t)(hScroll&0x07);
    hScrollSMask=0x7f>>(7-a);
    hScrollEMask=0xfe<<a;
    #else
    hScrollSMask=0;
    hScrollEMask=0;
    #endif
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

#endif
