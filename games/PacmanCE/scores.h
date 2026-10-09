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

#define SCORE_ADDR(i)       (EE_BASE+1+((i)*6))

uint32_t scoreAt(uint8_t i){
  uint16_t a=SCORE_ADDR(i);
  uint32_t s=eeprom_read_byte((const uint8_t*)(a+2));
  s=(s<<8)|eeprom_read_byte((const uint8_t*)(a+1));
  s=(s<<8)|eeprom_read_byte((const uint8_t*)a);
  return s;
}

uint8_t nameCharAt(uint8_t i, uint8_t c){
  return eeprom_read_byte((const uint8_t*)(SCORE_ADDR(i)+3+c));
}

void writeScore(uint8_t i, uint32_t score, const uint8_t *name){
  uint16_t a=SCORE_ADDR(i);
  eeprom_update_byte((uint8_t*)a,score&0xff);
  eeprom_update_byte((uint8_t*)(a+1),(score>>8)&0xff);
  eeprom_update_byte((uint8_t*)(a+2),(score>>16)&0xff);
  for(uint8_t c=0;c<3;c++){
    eeprom_update_byte((uint8_t*)(a+3+c),name[c]);
  }
}

/// @brief Forget the table, so that scoresInit() puts the original scores back
void scoresReset(){
  eeprom_update_byte((uint8_t*)EE_BASE,0);
}

/// @brief Set up the table the first time the game runs
void scoresInit(){
  if(eeprom_read_byte((const uint8_t*)EE_BASE)==EE_MAGIC){
    return;
  }
  for(uint8_t i=0;i<NUM_SCORES;i++){
    uint8_t name[3];
    for(uint8_t c=0;c<3;c++){
      name[c]=pgm_read_byte(&defaultNames[i][c]);
    }
    writeScore(i,pgm_read_dword(&defaultScores[i]),name);
  }
  eeprom_update_byte((uint8_t*)EE_BASE,EE_MAGIC);
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
}

#endif
