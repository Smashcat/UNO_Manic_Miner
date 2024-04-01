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

// Functions only used for debugging of game. Not compiled for normal use...

#include "engine.h"
/*
void showDebug(){
  Serial.begin(115200);
  for(uint16_t n=200;n<300;n+=8){
    Serial.print(n>>3);
    Serial.print(" = ");
    Serial.println( 4+((n>>3)%(CHARACTER_ROWS-4)) );  
  }
  while(1);
}

void printBinary(uint8_t x){
  for(int8_t n=7;n>-1;n--){
    Serial.print( x&(1<<n)?"@":".");
  }
}
*/

