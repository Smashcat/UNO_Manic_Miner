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

// Update input hold buffer. If something has changed, returns bitfield of changed buttons
// Note that a LOW bit = button pressed
// Order of bits from LSb->MSb = "B","A","Down","Up","Right","Left","Start","Option" (Start not yet connected)
// First 6 inputs are on PORTC, Option on PORTD2, Start on PORTD0
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
