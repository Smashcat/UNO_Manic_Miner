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

// The high score table, kept in EEPROM. Each of the 10 entries is a 24 bit score (3 bytes, low byte first) followed by
// a 3 character name. Names are stored as letter numbers: 0=A ... 25=Z, 26=space. A marker byte at the start shows
// whether the table has ever been set up.
//
// The EEPROM must not be touched while the picture is being drawn: the video interrupt uses the EEPROM address and data registers
// as scratch storage on every scanline, so an access made at any other time than between frames gets the wrong address or data
// (garbled scores, and writes that go to the wrong place). So the table lives in RAM while it is in use (in the same memory as the
// maze's dot flags, which are not needed on the screens that show the table), is read from the EEPROM in one go between frames,
// and changes are written back to the EEPROM a byte at a time, one per frame, in the gap between frames.

#ifndef __SCORES
#define __SCORES

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "gameDefs.h"

// The scores the original game starts with
const uint8_t defaultNames[NUM_SCORES][3] PROGMEM={
  {15,0,2},{0,10,0},{15,8,13},{0,14,18},{6,20,25},{18,20,4},{21,0,11},{0,11,1},{13,0,12},{2,2,2}
};
const uint32_t defaultScores[NUM_SCORES] PROGMEM={76500,59000,55000,52000,49000,45000,39000,16300,7650,4230};

#define SCORE_IX(i)         (1+((i)*6))              // position of an entry in the table (the marker byte is first)
#define TABLE_BYTES         (1+(NUM_SCORES*6))
#define VBLANK_START        (BOTTOM_EDGE+2)          // from this scan line the video interrupt is finished with the frame
#define BLANK_LINES_WRITE   57                       // scan lines of gap needed to write a byte (3.4ms is 54 lines)
#define BLANK_LINES_READ    12

uint8_t *const eeCache=(uint8_t*)dotBits;           // the table, in RAM
uint8_t eeNext=TABLE_BYTES+1;                        // next byte to write back to the EEPROM (TABLE_BYTES+1 = all done)
uint8_t scoresCached=0;                              // is the table in RAM (it is lost when a game starts)

/// @brief How many scan lines are left before the video interrupt starts drawing the next frame (0 if it is drawing now)
uint8_t blankLinesLeft(){
  uint16_t s;
  do{
    s=scanline;
  }while(s!=scanline);
  if(s>=VBLANK_START){
    return (312-s)+TOP_EDGE;
  }
  return (s<TOP_EDGE)?(TOP_EDGE-s):0;
}

/// @brief Get the table into RAM (from the EEPROM, unless it is already there)
void scoresLoad(){
  if(scoresCached){
    return;
  }
  while(blankLinesLeft()<BLANK_LINES_READ){
  }
  for(uint8_t i=0;i<TABLE_BYTES;i++){
    eeCache[i]=eeprom_read_byte((const uint8_t*)(EE_BASE+i));
  }
  scoresCached=1;
}

uint32_t scoreAt(uint8_t i){
  scoresLoad();
  const uint8_t *a=eeCache+SCORE_IX(i);
  return (((uint32_t)a[2])<<16)|(((uint32_t)a[1])<<8)|a[0];
}

uint8_t nameCharAt(uint8_t i, uint8_t c){
  scoresLoad();
  return eeCache[SCORE_IX(i)+3+c];
}

void writeScore(uint8_t i, uint32_t score, const uint8_t *name){
  uint8_t *a=eeCache+SCORE_IX(i);
  a[0]=score&0xff;
  a[1]=(score>>8)&0xff;
  a[2]=(score>>16)&0xff;
  for(uint8_t c=0;c<3;c++){
    a[3+c]=name[c];
  }
}

/// @brief Called every frame, as soon as the frame has finished drawing: copies one changed byte of the table back to the EEPROM (the
/// marker, which says the table is good, goes last)
void scoresFlush(){
  if(eeNext>TABLE_BYTES){
    return;
  }
  // (the frame has only just finished: wait the few scan lines until the video interrupt is completely done with it)
  while(blankLinesLeft()<BLANK_LINES_WRITE){
  }
  while(eeNext<=TABLE_BYTES){
    uint8_t k=(eeNext==TABLE_BYTES)?0:eeNext;
    if(eeprom_read_byte((const uint8_t*)(EE_BASE+k))!=eeCache[k]){
      if(blankLinesLeft()<BLANK_LINES_WRITE){
        return;      // (not enough gap left: the next frame carries on)
      }
      eeprom_write_byte((uint8_t*)(EE_BASE+k),eeCache[k]);
      eeprom_busy_wait();
      ++eeNext;
      return;
    }
    ++eeNext;
  }
}

/// @brief Finish copying the table back to the EEPROM (before something else uses the RAM it is held in)
void scoresFlushAll(){
  while(eeNext<=TABLE_BYTES){
    waitFrame();
    scoresFlush();
  }
}

/// @brief Set up the table the first time the game runs (or put the original scores back if "reset" is set)
void scoresInit(uint8_t reset){
  while(blankLinesLeft()<BLANK_LINES_READ){
  }
  if(!reset && eeprom_read_byte((const uint8_t*)EE_BASE)==EE_MAGIC){
    return;
  }
  for(uint8_t i=0;i<NUM_SCORES;i++){
    uint8_t name[3];
    for(uint8_t c=0;c<3;c++){
      name[c]=pgm_read_byte(&defaultNames[i][c]);
    }
    writeScore(i,pgm_read_dword(&defaultScores[i]),name);
  }
  eeCache[0]=EE_MAGIC;
  scoresCached=1;
  eeNext=1;
}

/// @brief Where would this score go in the table? NUM_SCORES means it doesn't make it
uint8_t scoreRank(uint32_t score){
  for(uint8_t i=0;i<NUM_SCORES;i++){
    if(score>scoreAt(i)){
      return i;
    }
  }
  return NUM_SCORES;
}

/// @brief Put a new score into the table at the position scoreRank() gave for it, pushing the others down
void insertScore(uint8_t rank, uint32_t score, const uint8_t *name){
  for(uint8_t i=NUM_SCORES-1;i>rank;i--){
    uint8_t n[3];
    for(uint8_t c=0;c<3;c++){
      n[c]=nameCharAt(i-1,c);
    }
    writeScore(i,scoreAt(i-1),n);
  }
  writeScore(rank,score,name);
  eeNext=1;      // (and now the EEPROM has to be brought up to date)
}

#endif
