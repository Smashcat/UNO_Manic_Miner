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
#define SFX_PELLET      2     // a power pellet: three times as long as a chomp, and a dot eaten straight after cannot cut it short
#define SFX_FRUIT       3
#define SFX_GHOST       4
#define SFX_DEATH       5     // takes over both channels

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
  setPWMChannelBaseSetting(1,FREQ256);   // /256, so channel 1 reaches down to about 120Hz
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
      break;
      case SFX_PELLET:
        // Three times as long as a chomp, an octave lower: the pitch slides up, then down, then up again (a smaller OCR value is a
        // higher note, so "up" counts the value down)
        ocr=(((f&4)?(64+((f&3)*8)):(88-((f&3)*8)))<<1)+1;
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
  if(extraLifeTimer){
    // The extra life jingle: a run of dings, each a short high note that drops a little, with a one frame gap after it. The last
    // one has no gap and is held on for a couple of frames more
    uint16_t f=EXTRA_LIFE_TOTAL-extraLifeTimer;
    uint8_t k=f%EXTRA_LIFE_DING_FRAMES;
    --extraLifeTimer;
    uint8_t lastNote=14+((EXTRA_LIFE_DING_FRAMES-2)>>1);
    if(f>=(EXTRA_LIFE_TOTAL-EXTRA_LIFE_HOLD-1)){
      setPWMChannelFreq(1,lastNote);
    }else{
      setPWMChannelFreq(1,(k<(EXTRA_LIFE_DING_FRAMES-1))?(14+(k>>1)):0);
    }
    return;
  }
  bool eyes=false,scared=false;
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
    }
  }
  if(eyes){
    ocr=(tick&2)?17:25;
  }else if(scared){
    ocr=75-((tick&7)*2);
  }else{
    // The siren, always there while the ghosts are not frightened: a low "whoo, whoo" as the pitch glides up and down.
    // It cycles faster the further into the game you are (as the ghosts get faster)
    sirenPhase+=SIREN_SPEED_BASE+(mapSeq<SIREN_SPEED_MAX?mapSeq:SIREN_SPEED_MAX);
    uint8_t t=(sirenPhase<128)?sirenPhase:(255-sirenPhase);    // a triangle wave, 0 to 127
    ocr=150-(t>>1);                                            // about 207Hz up to 355Hz
  }
  setPWMChannelFreq(1,ocr);
}

// ---- Start-of-game music ---------------------------------------------------------------------------------------
// Melody on channel 0 and a simple bass line on channel 1. Each entry is a timer value for the channel (0 = silent)
// and a length in frames.

#define INTRO_NOTES     29

const uint8_t introMelody[INTRO_NOTES] PROGMEM={
  62,31,41,49,31,41,49,
  59,29,39,46,29,39,46,
  62,31,41,49,31,41,49,
  49,46,44,41,39,37,31,
  0
};
const uint8_t introBass[INTRO_NOTES] PROGMEM={
  252,252,252,252,252,252,252,
  238,238,238,238,238,238,238,
  252,252,252,252,252,252,252,
  200,200,200,189,189,189,168,
  0
};
const uint8_t introLen[INTRO_NOTES] PROGMEM={
  6,6,6,6,6,6,12,
  6,6,6,6,6,6,12,
  6,6,6,6,6,6,12,
  6,6,6,6,6,6,18,
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
