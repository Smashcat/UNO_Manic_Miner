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

extern "C" { void renderScanline(volatile uint8_t *pScreenRam, volatile const uint8_t *fontSlice, volatile uint16_t tcnt, volatile uint16_t minTCNT); }
extern "C" { void loadHi(); }

void unpackBytes(uint8_t *dst, const uint8_t *conversionTable, const uint8_t *src, uint16_t len, uint8_t bits){
    uint8_t mask=(0xff>>(8-bits));
    uint8_t bP=0;
    uint16_t ix=0;
    uint8_t b=0;
    uint8_t v=0;
    for(uint16_t n=0;n<len;n++){
        b=pgm_read_byte(src+ix);
        v=(b&(mask<<bP))>>bP;
        bP+=bits;
        if(bP>=8 && n<len-1){
            ++ix;
            b=pgm_read_byte(src+ix);
            uint8_t v2=(b&~( 0xff << (bP-8) ))<<(8-(bP-bits));
            v|=v2;
            bP-=8;
        }
        if(conversionTable!=NULL){
          *dst++=pgm_read_byte(conversionTable+v);
        }else{
          *dst++=v;
        }
    }
}

void initHardware(){

  spriteDefRef=spriteDef+0;
  fontIX=pgm_read_byte(&gameHeader.fontIX);
  bigFontIX=pgm_read_byte(&gameHeader.bigFontIX);
  clearScreen(0,32,190);
  setTileRowSplit(224);
  loadHi(); // Load from EEPROM before we start the video output, as we steal some of the EEPROM registers for use in the rendering code - we can still store to EEPROM, but need to ensure there is no clash :)

  // prime pixel suppressor - when Pin is switched to output, it will force BLACK
  // DDRD&=~(1<<PIN_SUPPRESS); - input is the default
  // PORTD&=~(1<<PIN_SUPPRESS); - low is teh default
  
  setAudioType(muted);

  // Set timer 2 for PWM output on D11 (PORTB bit 3)
  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS21) | _BV(CS20);

  // Set timer 0 for output on D6
  TCCR0A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM01) | _BV(WGM00);
  TCCR0B = _BV(WGM22) | _BV(CS01) | _BV(CS00);

  // Configure input pins
  PORTC=0b111111; // pullups enabled
  PORTD|=(1<<2);     // Option button on D2
  PORTB|=(1<<0);     // Start button on B0
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
  //while(currentFrame==lastFrame);
  //lastFrame=currentFrame;
}
