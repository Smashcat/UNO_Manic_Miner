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

// Sound for Pac-Man Championship. See the notes below on how the two channels are used.

#ifndef __AUDIO
#define __AUDIO

#include "gameVars.h"

// ---- Audio -----------------------------------------------------------------------------------------------------
// Two square wave channels, updated once per frame (50Hz).
//   Channel 0 (Timer 0, divide by 256): sound effects - the "waka" chomp, ghost eaten, fruit and the death sound.
//   Channel 1 (Timer 2, divide by 128): the ghost siren, which changes to a warble when the ghosts are frightened and a
//                                       fast chirp when eyes are returning to the box. Silent when no ghost is out.
// Output frequency is 16MHz / (2 * divider * (OCR+1)), so a bigger value is a lower note.

#define SFX_NONE        0
#define SFX_CHOMP       1
#define SFX_FRUIT       2
#define SFX_GHOST       3
#define SFX_DEATH       4     // takes over both channels

void sfxStart(uint8_t type, uint8_t frames){
  // A new sound only interrupts one of lower or equal priority
  if(sfxTimer==0 || type>=sfxType){
    sfxType=type;
    sfxTimer=frames;
    sfxFrame=0;
  }
}

void audioInit(){
  setPWMChannelBaseSetting(0,_BV(CS02));   // /256
  setPWMChannelBaseSetting(1,FREQ128);
  setPWMChannelFreq(0,0);
  setPWMChannelFreq(1,0);
  sfxTimer=0;
  sfxType=SFX_NONE;
}

void audioMute(){
  sfxTimer=0;
  setPWMChannelFreq(0,0);
  setPWMChannelFreq(1,0);
}

/// @brief Called once a frame while the game is running
void audioUpdate(){
  // Channel 0 - effects
  uint8_t ocr=0;
  if(sfxTimer){
    uint8_t f=sfxFrame++;
    --sfxTimer;
    switch(sfxType){
      case SFX_CHOMP:
        // Alternating rising and falling chirps, so eating dots goes "waka waka"
        ocr=(sfxToggle?(88-(f*8)):(60+(f*8)));
        if(sfxLow){
          ocr=(ocr<<1)+1;   // power pellets sound an octave lower
        }
      break;
      case SFX_FRUIT:
        ocr=(f<10)?50:34;
      break;
      case SFX_GHOST:
        ocr=90-(f*5);
      break;
      case SFX_DEATH:
      {
        // Pitch drifts down while wobbling up and down - Pac-Man deflating
        uint8_t w=f%12;
        ocr=24+(f/2)+(((w<6)?w:(11-w))*3);
      }
      break;
    }
  }
  setPWMChannelFreq(0,ocr);

  // Channel 1 - ghosts
  ocr=0;
  if(sfxTimer && sfxType==SFX_DEATH){
    setPWMChannelFreq(1,0);
    return;
  }
  bool eyes=false,scared=false,out=false;
  for(uint8_t i=0;i<NUM_GHOSTS;i++){
    switch(ghosts[i].state){
      case GS_EYES:
      case GS_ENTER:
        eyes=true;
      break;
      case GS_PEN:
      case GS_LEAVING:
        if(scareTimer){ scared=true; }
      break;
      case GS_SCARED:
        scared=true;
      break;
      case GS_AWAKE:
        out=true;
      break;
    }
  }
  if(eyes){
    ocr=(tick&2)?36:52;
  }else if(scared){
    ocr=150-((tick&7)*5);
  }else if(out){
    // Siren: slowly rising and falling
    uint8_t p=tick&63;
    ocr=150-(((p<32)?p:(63-p))*2);
  }
  setPWMChannelFreq(1,ocr);
}

// ---- Start-of-game music ---------------------------------------------------------------------------------------
// Melody on channel 0 and a simple bass line on channel 1. Each entry is a timer value for the channel (0 = silent)
// and a length in frames.

#define INTRO_NOTES     30

const uint8_t introMelody[INTRO_NOTES] PROGMEM={
  62,31,41,49,31,41,49,
  59,29,39,46,29,39,46,
  62,31,41,49,31,41,49,
  49,46,44,41,39,37,35,31,
  0
};
const uint8_t introBass[INTRO_NOTES] PROGMEM={
  252,252,252,252,252,252,252,
  238,238,238,238,238,238,238,
  252,252,252,252,252,252,252,
  200,200,200,189,189,189,178,168,
  0
};
const uint8_t introLen[INTRO_NOTES] PROGMEM={
  6,6,6,6,6,6,12,
  6,6,6,6,6,6,12,
  6,6,6,6,6,6,12,
  6,6,6,6,6,6,6,18,
  30
};

/// @brief Play the next frame of the start music
/// @return true when it has finished
bool introUpdate(){
  if(introIx>=INTRO_NOTES){
    return true;
  }
  if(introTimer==0){
    introTimer=pgm_read_byte(&introLen[introIx]);
  }
  uint8_t m=pgm_read_byte(&introMelody[introIx]);
  uint8_t b=pgm_read_byte(&introBass[introIx]);
  if(introTimer==1){
    m=0;    // a brief gap so repeated notes are separate
  }
  setPWMChannelFreq(0,m);
  setPWMChannelFreq(1,b);
  if(--introTimer==0){
    ++introIx;
  }
  return false;
}

#endif
