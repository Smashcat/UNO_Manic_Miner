#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\examples\\BasicScrollingTilemap\\videoOutput.S"
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

#include "engineDefs.h"

.equ   DDRD, 0x0A
.equ UCSR0B, 0xC1
.equ   UDR0, 0xC6

// Make use of some of the IO registers we don't need for anything else here for single clock IN/OUT storage
.equ GPIOR0, 0x1E
.equ GPIOR1, 0x2A
.equ GPIOR2, 0x2B
.equ EEDR,   0x20
.equ EEARL,  0x21
.equ EEARH,  0x21
.equ EECR,   0x1F

#define UNSUPPRESS CBI  DDRD, PIN_SUPPRESS
#define SUPPRESS   SBI  DDRD, PIN_SUPPRESS

 .global storeInEEPROM ; ()
storeInEEPROM:
  SBIC EECR,1 ; EEPE bit
  RJMP storeInEEPROM
  OUT EEARL,R20
  OUT EEDR,R21 ; data register port
  SBI EECR,2 ; EEMPE bit (master write enable)
  SBI EECR,1 ; EEPE bit (start write)
  RET

 .global storeHi ; ()
storeHi:
  EOR R21,R21
  LDI R20,0
  OUT EEARH,R20 ; always set high address byte to zero
  LDS R21,hiScore
  RCALL storeInEEPROM
  LDS R21,hiScore+1
  INC R20
  RCALL storeInEEPROM
  RET

 .global loadHi ;()
loadHi:
  LDI R20,0
  OUT EEARH, R20
  OUT EEARL, R20
  SBI EECR,0  ; EERE read bit  
  IN R21,EEDR
  STS hiScore,R21
  INC R20
  OUT EEARL,R20
  SBI EECR,0  ; EERE read bit  
  IN R21,EEDR
  STS hiScore+1,R21
  RET
  


 .global processSprite ;(ProcessedSprite *dest, const uint8_t *spriteDefRef, const SpriteData *spriteData)
 ;                                R25 R24                               R23 R22                         R21 R20

processSprite:
  PUSH R29
  PUSH R28
  MOVW R30,R20;     // Move sprite instance data pointer into Z
  LDI  R20,1
  EOR  R21,R21 
  MOVW R28,R24;     // Move the sprite graphical data pointer into Y
  
  LD R18,Z+         // Sprite Y pos
  ST Y+,R18         // Just copy Y pos into processed sprite data - no need to do anything with it here
  
  LD R18,Z+         // Sprite X pos in R18
  LDS R19,hScroll   // Horizontal scroll is a 16bit value, with lower 3 bits used as bit offset on display
  MOV R24,R19       // Duplicate into R24
  ANDI R24,0x07     // Keep the bottom 3 bits of X position to shift sprite data right in destination
  ADD R18,R24       // Add horizontal scroll offset from sprite X
  MOV R19,R18       // Duplicate into R19
  LSR R19           // Divide Sprite X pos by 8
  LSR R19
  LSR R19           
  ST Y+,R19         // Byte position in line buffer for sprite now in processed sprite data
                    // Y now points to the data buffer
  ANDI R18,0x07     // Keep the bottom 3 bits of X position to shift sprite data right in destination
  
  LD   R19,Z+       // sprite definition index
  MOVW R30,R22      // Base pointer for sprite defs - now R23:R22 free
  LDI  R22,32       // Multiply the sprite index by 32
  PUSH R1
  PUSH R0
  MUL  R19,R22
  ADD  R30,R0        // Add the sprite index * 32 to the base pointer address
  ADC  R31,R1
  POP  R0
  POP  R1
  
  ; //Now loop through the sprite graphical data, dropping it into the processed sprite data, which is 3 x 16 bytes to allow for shifting
  ; // Y is pointing to the destination array
  ; // Z is pointing to the graphical data
  ; // R22 is pointing to the source array offset in program memory
  ; // R18 is the number of bits to shift right
  LDI R19,16        // 16 rows of data

lineLoop:
  LPM R24,Z+        // Load from program memory
  ST Y,R24
  LPM R24,Z+
  STD Y+16,R24
  STD Y+32,R21      // zero out the 3rd byte in the row, so we're not showing old data that's not rotated out!
  CP R18,R21        // Check number of bits to shift against zero
  BREQ skipShift    // Nothing to shift?

  MOV R24,R18       // Number of bits to shift
  CLC               // Clear the carry flag - no need to do it in loop, as the DEC resets it

shiftLoop:          // Shift sprite graphical data ro the right by the horizontal position &0x07 (12 clocks per loop)
  LD R25,Y
  ROR R25   
  ST Y,R25
  
  LDD R25,Y+16
  ROR R25
  STD Y+16,R25
  
  LDD R25,Y+32
  ROR R25
  STD Y+32,R25
  
  DEC R24
  BRNE shiftLoop
  
skipShift:
  ADD R28,R20       // Move Y to point to next destination
  ADC R29,R21
  DEC R19
  BRNE lineLoop;

  POP R28
  POP R29
  RET










/**
 * This is where the current scanline is rasterised to the display
 */

 .global renderScanline ; (*pScreenRam, *fontSlice, tcnt,   minTCNT)
 ;                       R25  R24     R23 R22    R21 R20 R19 R18

 renderScanline:
	; interrupt may jitter because of instructions taking 1, 2, 3 clock cycles
	; (and maybe more if there are other interrupts)
	; this hack synchronizes the CPU with the tcnt value
	; only checking the low byte here because interrupt should always occur earlier than 240 counts (15 us)
  
	INC R18
	CP R20, R18 ; compare tcnt with minTCNT+1
	BRLO jitterFix1 ; extra cycle if tcnt was lower
 
jitterFix1:
	INC R18
	CP R20, R18 ; compare tcnt with minTCNT+2
	BRLO jitterFix2
 
jitterFix2:
	INC R18
	CP R20, R18 ; compare tcnt with minTCNT+3
	BRLO jitterFix3
 
jitterFix3:


  SUPPRESS  // Prevent UART initially sending a high signal to the output when its enabled by pulling the suppress pin low

  MOVW R26, R24 ; X = pScreenRam
  LDI R25, 0x08
#if USE_MASK
  LDS R18,hScrollSMask;
#else
  LDI R18,0xFF
#endif






  // Lots of loop unrolling here. We use almost all of the time in between loading the UART to get the data ready for the next byte, clearing the line-buffer,
  // and also preparing some data for the line-buffer generation after the tiles+previous line-buffer have all been sent to the display. There
  // are only 4 NOPs left which is obviously a TRAVESTY! but there is really nothing else I could use them for currently.
  
  LD R24, X+ ; get character from screen ram // +2 =  2
  MOVW R30, R22 ; Z = fontSlice              // +1 =  3
  ADD R30, R24                               // +1 =  4
  LPM R20, Z ; fetch 8 pixels                // +3 =  7
  LDS R24,spriteLine ; get lineBuffer byte   // +2 =  9
  OR R20, R24 ; OR the lineBuffer            // +1 = 10
  AND R20,R18 ; Mask off X pixels, based on the current hScroll offset

  // R19:R18 pair free to use till R19 is set to zero
  
  STS UDR0, R20 ; send them to the USART     // +2 = 14
  
  LD R24, X+ ; get character from screen ram // +2 = 19 - had to swap things about to ensure UART kept fed!
  MOVW R30, R22 ; Z = fontSlice              // +1 = 20
  PUSH R29  ;// Store Y (high byte)

  STS UCSR0B, R25 ; enable USART transmit    // +2 = 16

  UNSUPPRESS                                 // +1 = 17
  PUSH R28  ; // Store Y (low byte)                                                                       *
  LDI R29,hi8(processedSprite+PROCESSED_SPRITE_OFFSET)           // Load processed sprite data array address into Y register *
    
  ADD R30, R24                               // +1 = 21
  LPM R18, Z ; fetch 8 pixels                // +3 = 24
  LDS R24,spriteLine+1 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 27
  STS UDR0, R18 ; send them to the USART     // +2 = 30

  PUSH R17                                   //                                                           *
  LDI R28,lo8(processedSprite+PROCESSED_SPRITE_OFFSET)           //                                                          *
  
  LD R24, X+ ; get character from screen ram // +2 = 35
  MOVW R30, R22 ; Z = fontSlice              // +1 = 36
  ADD R30, R24                               // +1 = 37
  LPM R18, Z ; fetch 8 pixels                // +3 = 40
  LDS R24,spriteLine+2 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 43

  PUSH R16                                   //                                                           *
  
  STS UDR0, R18 ; send them to the USART     // +2 = 45
  
  LD R24, X+ ; get character from screen ram // +2 = 49
  MOVW R30, R22 ; Z = fontSlice              // +1 = 50
  ADD R30, R24                               // +1 = 51
  LPM R18, Z ; fetch 8 pixels                // +3 = 54
  LDS R24,spriteLine+3 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 57
  
  LDS R20,displayLine       ; R20 now holds the current display line (0 - 8*31) +2                        *
  
  STS UDR0, R18 ; send them to the USART     // +2 = 61
  ;// Quad block end

  ; // Quad block start
  LD R24, X+ ; get character from screen ram // +2 =  2
  MOVW R30, R22 ; Z = fontSlice              // +1 =  3
  ADD R30, R24                               // +1 =  4
  LPM R18, Z ; fetch 8 pixels                // +3 =  7
  LDS R24,spriteLine+4 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 10

  NOP
  NOP
        
  STS UDR0, R18 ; send them to the USART     // +2 = 13
  
  LD R24, X+ ; get character from screen ram // +2 = 17
  MOVW R30, R22 ; Z = fontSlice              // +1 = 18
  ADD R30, R24                               // +1 = 19
  LPM R18, Z ; fetch 8 pixels                // +3 = 22
  LDS R24,spriteLine+5 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 25

  NOP
  NOP
    
  STS UDR0, R18 ; send them to the USART     // +2 = 29

  MOV  R16,R20             ; move current scanline into R16                                           +1 *
  LD   R17,Y               ; sprite Y position                                                        +2 *
  SUB  R16,R17             ; Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
      
  LD R24, X+ ; get character from screen ram // +2 = 35
  MOVW R30, R22 ; Z = fontSlice              // +1 = 36
  ADD R30, R24                               // +1 = 37
  LPM R18, Z ; fetch 8 pixels                // +3 = 40
  LDS R24,spriteLine+6 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 43
  STS UDR0, R18 ; send them to the USART     // +2 = 45

  OUT EEARL,R16            ;                                                                          +1 *
  SBIW R28,50              ; Move back through processed Sprite array                                 +1 *
    
  LD R24, X+ ; get character from screen ram // +2 = 49
  MOVW R30, R22 ; Z = fontSlice              // +1 = 50
  ADD R30, R24                               // +1 = 51
  LPM R18, Z ; fetch 8 pixels                // +3 = 54
  LDS R24,spriteLine+7 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 57

  LD   R17,Y               ; sprite Y position                                                        +2 *
  
  STS UDR0, R18 ; send them to the USART     // +2 = 61
  ;// Quad block end

  ; // Quad block start - byte 8
  LD R24, X+ ; get character from screen ram // +2 =  2
  MOVW R30, R22 ; Z = fontSlice              // +1 =  3
  ADD R30, R24                               // +1 =  4
  LPM R18, Z ; fetch 8 pixels                // +3 =  7
  LDS R24,spriteLine+8 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 10

  MOV  R16,R20             ; move current scanline into R18                                           +1 *
  SUB  R16,R17             ; Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
  
  STS UDR0, R18 ; send them to the USART     // +2 = 13
  
  LD R24, X+ ; get character from screen ram // +2 = 17
  MOVW R30, R22 ; Z = fontSlice              // +1 = 18
  ADD R30, R24                               // +1 = 19
  LPM R18, Z ; fetch 8 pixels                // +3 = 22
  LDS R24,spriteLine+9 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 25

  OUT EEDR,R16             ; store in special register                                                +1 *
  EOR R19,R19 ; // R19 now used to clear sprite linebuffer
  
  STS UDR0, R18 ; send them to the USART     // +2 = 29

  SBIW R28,50              ; Move back through processed Sprite array                                 +1 *
  MOV  R16,R20             ; move current scanline into R18                                           +1 *
          
  LD R24, X+ ; get character from screen ram // +2 = 35
  MOVW R30, R22 ; Z = fontSlice              // +1 = 36
  ADD R30, R24                               // +1 = 37
  LPM R18, Z ; fetch 8 pixels                // +3 = 40
  LDS R24,spriteLine+10 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 43
  STS UDR0, R18 ; send them to the USART     // +2 = 45

  LD   R17,Y               ; sprite Y position                                                        +2 *
    
  LD R24, X+ ; get character from screen ram // +2
  MOVW R30, R22 ; Z = fontSlice              // +1
  ADD R30, R24                               // +1
  LPM R18, Z ; fetch 8 pixels                // +3
  LDS R24,spriteLine+11 ; get lineBuffer byte // +2
  OR R18, R24 ; OR the lineBuffer            // +1
  
  SUB  R16,R17             ; Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
  SBIW R28,50              ; Move back through processed Sprite array                                 +1 *
  
  STS UDR0, R18 ; send them to the USART     // +2
  ;// Quad block end

  OUT GPIOR2,R16             ; store in special register                                              +1 *
  MOV  R16,R20             ; move current scanline into R16                                           +1 *
  
  ; // Quad block start
  LD R24, X+ ; get character from screen ram // +2 =  2
  MOVW R30, R22 ; Z = fontSlice              // +1 =  3
  ADD R30, R24                               // +1 =  4
  LPM R18, Z ; fetch 8 pixels                // +3 =  7
  LDS R24,spriteLine+12 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 10
  STS spriteLine+12,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 13

  STS spriteLine+11,R19  ; Clear linebuffer  // +2

  LD R24, X+ ; get character from screen ram // +2
  MOVW R30, R22 ; Z = fontSlice              // +1
  ADD R30, R24                               // +1
  LPM R18, Z ; fetch 8 pixels                // +3
  LDS R24,spriteLine+13 ; get lineBuffer byte //+2
  OR R18, R24 ; OR the lineBuffer            // +1
  STS spriteLine+13,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 29

  STS spriteLine+10,R19  ; Clear linebuffer  // +2
  
  LD R24, X+ ; get character from screen ram // +2 = 35
  MOVW R30, R22 ; Z = fontSlice              // +1 = 36
  ADD R30, R24                               // +1 = 37
  LPM R18, Z ; fetch 8 pixels                // +3 = 40
  LDS R24,spriteLine+14 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 43
  STS spriteLine+14,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 45

  STS spriteLine+9,R19  ; Clear linebuffer  // +2
  
  LD R24, X+ ; get character from screen ram // +2 = 49
  MOVW R30, R22 ; Z = fontSlice              // +1 = 50
  ADD R30, R24                               // +1 = 51
  LPM R18, Z ; fetch 8 pixels                // +3 = 54
  LDS R24,spriteLine+15 ; get lineBuffer byte // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 57
  STS spriteLine+15,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 61
  ;// Quad block end

  STS spriteLine+8,R19  ; Clear linebuffer  // +2
  
  ; // Quad block start
  LD R24, X+ ; get character from screen ram // +2 =  2
  MOVW R30, R22 ; Z = fontSlice              // +1 =  3
  ADD R30, R24                               // +1 =  4
  LPM R18, Z ; fetch 8 pixels                // +3 =  7
  LDS R24,spriteLine+16 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 10
  STS spriteLine+16,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 13
  
  STS spriteLine+7,R19  ; Clear linebuffer  // +2
  
  LD R24, X+ ; get character from screen ram // +2 = 17
  MOVW R30, R22 ; Z = fontSlice              // +1 = 18
  ADD R30, R24                               // +1 = 19
  LPM R18, Z ; fetch 8 pixels                // +3 = 22
  LDS R24,spriteLine+17 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 25
  STS spriteLine+17,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 29

  STS spriteLine+6,R19  ; Clear linebuffer  // +2
    
  LD R24, X+ ; get character from screen ram // +2 = 35
  MOVW R30, R22 ; Z = fontSlice              // +1 = 36
  ADD R30, R24                               // +1 = 37
  LPM R18, Z ; fetch 8 pixels                // +3 = 40
  LDS R24,spriteLine+18 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 43
  STS spriteLine+18,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 45

  STS spriteLine+5,R19  ; Clear linebuffer  // +2
  
  LD R24, X+ ; get character from screen ram // +2 = 49
  MOVW R30, R22 ; Z = fontSlice              // +1 = 50
  ADD R30, R24                               // +1 = 51
  LPM R18, Z ; fetch 8 pixels                // +3 = 54
  LDS R24,spriteLine+19 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 57
  STS spriteLine+19,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 61
  ;// Quad block end

  STS spriteLine+4,R19  ; Clear linebuffer  // +2
  
  ; // Quad block start
  LD R24, X+ ; get character from screen ram // +2 =  2
  MOVW R30, R22 ; Z = fontSlice              // +1 =  3
  ADD R30, R24                               // +1 =  4
  LPM R18, Z ; fetch 8 pixels                // +3 =  7
  LDS R24,spriteLine+20 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 10
  STS spriteLine+20,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 13

  STS spriteLine+3,R19  ; Clear linebuffer  // +2
  
  LD R24, X+ ; get character from screen ram // +2 = 17
  MOVW R30, R22 ; Z = fontSlice              // +1 = 18
  ADD R30, R24                               // +1 = 19
  LPM R18, Z ; fetch 8 pixels                // +3 = 22
  LDS R24,spriteLine+21 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 25
  STS spriteLine+21,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 29

  STS spriteLine+2,R19  ; Clear linebuffer  // +2
    
  LD R24, X+ ; get character from screen ram // +2 = 35
  MOVW R30, R22 ; Z = fontSlice              // +1 = 36
  ADD R30, R24                               // +1 = 37
  LPM R18, Z ; fetch 8 pixels                // +3 = 40
  LDS R24,spriteLine+22 ; get lineBuffer byte// +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 43
  STS spriteLine+22,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 45

  STS spriteLine+1,R19  ; Clear linebuffer  // +2
  
  LD R24, X+ ; get character from screen ram // +2 = 49
  MOVW R30, R22 ; Z = fontSlice              // +1 = 50
  ADD R30, R24                               // +1 = 51
  LPM R18, Z ; fetch 8 pixels                // +3 = 54
  LDS R24,spriteLine+23 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 57
  STS spriteLine+23,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 61
  ;// Quad block end

  LD   R17,Y               ; sprite Y position                                                        +2 *

  ; // Quad block start
  LD R24, X+ ; get character from screen ram // +2 =  2
  MOVW R30, R22 ; Z = fontSlice              // +1 =  3
  ADD R30, R24                               // +1 =  4
  LPM R18, Z ; fetch 8 pixels                // +3 =  7
  LDS R24,spriteLine+24 ; get lineBuffer by  // +2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 10
  STS spriteLine+24,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 13

  SUB  R16,R17             ; Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
  SBIW R28,50              ; Move back through processed Sprite array                                 +1 *
  OUT GPIOR1,R16             ; store in special register                                              +1 *
  MOV  R16,R20             ; move current scanline into R16                                           +1 *
      
  LD R24, X+ ; get character from screen ram // +2 = 17
  MOVW R30, R22 ; Z = fontSlice              // +1 = 18
  ADD R30, R24                               // +1 = 19
  LPM R18, Z ; fetch 8 pixels                // +3 = 22
  LDS R24,spriteLine+25 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 25
  STS UDR0, R18 ; send them to the USART     // +2 = 29
  STS spriteLine+25,R19  ; Clear linebuffer  // +2
  
  LD R24, X+ ; get character from screen ram // +2 = 35
  MOVW R30, R22 ; Z = fontSlice              // +1 = 36
  ADD R30, R24                               // +1 = 37
  LPM R18, Z ; fetch 8 pixels                // +3 = 40
  LDS R24,spriteLine+26 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 43
  STS spriteLine+26,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 45

  LD   R17,Y               ; sprite Y position                                                        +2 *
  
  LD R24, X+ ; get character from screen ram // +2 = 49
  MOVW R30, R22 ; Z = fontSlice              // +1 = 50
  ADD R30, R24                               // +1 = 51
  LPM R18, Z ; fetch 8 pixels                // +3 = 54
  LDS R24,spriteLine+27 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 57
  STS spriteLine+27,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 61
  ;// Quad block end

  SUB  R16,R17             ; Check if sprite Y position is below current scanline (scanlineY-spriteY)   +1 *
  OUT GPIOR0,R16             ; store in special register                                                +1 *
    
  ; // Quad block start
  LD R24, X+ ; get character from screen ram // +2 =  2
  MOVW R30, R22 ; Z = fontSlice              // +1 =  3
  ADD R30, R24                               // +1 =  4
  LPM R18, Z ; fetch 8 pixels                // +3 =  7
  LDS R24,spriteLine+28 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 10
  STS spriteLine+28,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 13

  LDI R17,hi8(spriteLine)   ; Linebuffer address into R17:R16 register pair
  LDI R16,lo8(spriteLine)
  
  LD R24, X+ ; get character from screen ram // +2 = 17
  MOVW R30, R22 ; Z = fontSlice              // +1 = 18
  ADD R30, R24                               // +1 = 19
  LPM R18, Z ; fetch 8 pixels                // +3 = 22
  LDS R24,spriteLine+29 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 25
  STS spriteLine+29,R19  ; Clear linebuffer  // +2
  STS UDR0, R18 ; send them to the USART     // +2 = 29

  LDS R25,hScrollEMask;                      // To mask off extra pixels at end of tilemap based on the horizontal scroll position
  STS spriteLine,R19  ; Clear linebuffer  // +2

  LD R24, X+ ; get character from screen ram // +2 = 35
  MOVW R30, R22 ; Z = fontSlice              // +1 = 36
  ADD R30, R24                               // +1 = 37
  LPM R18, Z ; fetch 8 pixels                // +3 = 40
  LDS R24,spriteLine+30 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 43
  STS UDR0, R18 ; send them to the USART     // +2 = 45
  STS spriteLine+30,R19  ; Clear linebuffer  // +2

  EOR R21,R21                                // +1  R21:R20 = current scanline (R21 is always zero)
    
  LD R24, X+ ; get character from screen ram // +2 = 49
  MOVW R30, R22 ; Z = fontSlice              // +1 = 50
  ADD R30, R24                               // +1 = 51
  LPM R18, Z ; fetch 8 pixels                // +3 = 54
  LDS R24,spriteLine+31 ; get lineBuffer byte //+2 =  9
  OR R18, R24 ; OR the lineBuffer            // +1 = 57
  AND R18,R25                                // +1  AND the end of line mask
  STS UDR0, R18 ; send them to the USART     // +2 = 61
  STS spriteLine+31,R19  ; Clear linebuffer  // +2
  ;// Quad block end
  
  STS UCSR0B, R1 ; disable USART transmit
  
  // Note: The last pixel from a line MUST be LOW, otherwise analogue monitors will bleed across to right!
  // Can be avoided by also suppressing after about 26 clocks, but this wastes time...


;===========================================
; Now get sprites ready for next scanline...
;===========================================

  MOVW R24,R28            ; Store pointer to start of processedSprite data to reset Y for each sprite

  ; 19,23,30,31 = Free
  ; 18    = holds the data row from the sprite to display (0-15) after initial check
  ; 22    = counter for sprite rendering
  ; 21:20 = current scanline (R21 is always zero here)
  ; 17:16 = addr of displayLine
  ; 25:24 = Holds pointer to NEXT element in processedSprite data array
  ; 29:28 = pointer to processedSprite data (Y)
  ; 26:27 = points to displayLine to write line buffer (X)

  MOVW R28,R24            ; Store pointer to start of processedSprite data to reset Y for each sprite

#if NUM_SPRITES>0

  IN R18,GPIOR0
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine0          ; 16 or more pixels, so it's not displayed 
  
  LDD  R19,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into X
  ADD  R26,R19             ; Add X position along display line
  //ADC  R27,R21             ; So X is now the destination register ready to accept 3 bytes of sprite data
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21
#if OR_SPRITE0
  LDD  R19,Y+2               ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE0
  LDD  R19,Y+34            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#else
  LDD  R19,Y+2             ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE0
  LDD  R19,Y+34            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#endif

#endif

notOnLine0:

#if NUM_SPRITES>1

  ADIW R24,50

  IN R18,GPIOR1
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine1          ; 16 or more pixels, so it's not displayed 

  MOVW R28,R24
  LDD  R19,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R19             ; Add X position along display line
  //ADC  R27,R21             ; So Z is now the destination register ready to accept 3 bytes of sprite data
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21
  ; If no need to OR sprites (they never overlap, then we can save a LOT of cycles here!)
#if OR_SPRITE1
  LDD  R19,Y+2               ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE1
  LDD  R19,Y+34            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#else
  LDD  R19,Y+2             ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE1
  LDD  R19,Y+34            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#endif

#endif

notOnLine1:

#if NUM_SPRITES>2

  ADIW R24,50

  IN R18,GPIOR2
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine2          ; 16 or more pixels, so it's not displayed 

  MOVW R28,R24
  LDD  R19,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R19             ; Add X position along display line
  //ADC  R27,R21             ; So Z is now the destination register ready to accept 3 bytes of sprite data
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21
  ; If no need to OR sprites (they never overlap, then we can save a LOT of cycles here!)
#if OR_SPRITE2
  LDD  R19,Y+2               ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE2
  LDD  R19,Y+34            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#else
  LDD  R19,Y+2             ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE2
  LDD  R19,Y+34            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#endif

#endif

notOnLine2:

#if NUM_SPRITES>3

  ADIW R24,50

  IN R18,EEDR
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine3          ; 16 or more pixels, so it's not displayed 

  MOVW R28,R24
  LDD  R19,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R19             ; Add X position along display line
  //ADC  R27,R21             ; So Z is now the destination register ready to accept 3 bytes of sprite data
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21
  ; If no need to OR sprites (they never overlap, then we can save a LOT of cycles here!)
#if OR_SPRITE3
  LDD  R19,Y+2               ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE3
  LDD  R19,Y+34            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#else
  LDD  R19,Y+2             ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE3
  LDD  R19,Y+34            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#endif

#endif

notOnLine3:

#if NUM_SPRITES>4

  ADIW R24,50

  IN R18,EEARL
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine4          ; 16 or more pixels, so it's not displayed 
  
  MOVW R28,R24
  LDD  R19,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R19             ; Add X position along display line
  //ADC  R27,R21             ; So Z is now the destination register ready to accept 3 bytes of sprite data
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21
  ; If no need to OR sprites (they never overlap, then we can save a LOT of cycles here!)
#if OR_SPRITE4
  LDD  R19,Y+2             ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE4
  LDD  R19,Y+34            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#else
  LDD  R19,Y+2             ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE4
  LDD  R19,Y+34            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#endif

#endif

notOnLine4:

#if NUM_SPRITES>5

  ADIW R24,50

  MOVW R28,R24
  MOV  R18,R20             ; move current scanline into R18
  LD   R19,Y               ; sprite Y position
  SUB  R18,R19             ; Check if sprite Y position is below current scanline (scanlineY-spriteY)
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine5          ; 16 or more pixels, so it's not displayed 
  LDD  R19,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R19             ; Add X position along display line
  //ADC  R27,R21             ; So Z is now the destination register ready to accept 3 bytes of sprite data
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21
  ; If no need to OR sprites (they never overlap, then we can save a LOT of cycles here!)
#if OR_SPRITE5
  LDD  R19,Y+2               ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE5
  LDD  R19,Y+34            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#else
  LDD  R19,Y+2             ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE5
  LDD  R19,Y+34            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#endif

#endif

notOnLine5:

#if NUM_SPRITES>6

  ADIW R24,50

  MOVW R28,R24
  MOV  R18,R20             ; move current scanline into R18
  LD   R19,Y               ; sprite Y position
  SUB  R18,R19             ; Check if sprite Y position is below current scanline (scanlineY-spriteY)
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine6          ; 16 or more pixels, so it's not displayed 
  LDD  R19,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R19             ; Add X position along display line
  //ADC  R27,R21             ; So Z is now the destination register ready to accept 3 bytes of sprite data
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21
  ; If no need to OR sprites (they never overlap, then we can save a LOT of cycles here!)
#if OR_SPRITE6
  LDD  R19,Y+2               ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE6
  LDD  R19,Y+34            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#else
  LDD  R19,Y+2             ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE6
  LDD  R19,Y+34            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#endif

#endif 

notOnLine6:

#if NUM_SPRITES>7

  ADIW R24,50

  MOVW R28,R24
  MOV  R18,R20             ; move current scanline into R18
  LD   R19,Y               ; sprite Y position
  SUB  R18,R19             ; Check if sprite Y position is below current scanline (scanlineY-spriteY)
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine7          ; 16 or more pixels, so it's not displayed 
  LDD  R19,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into X
  ADD  R26,R19             ; Add X position along display line
  //ADC  R27,R21             ; So X is now the destination register ready to accept 3 bytes of sprite data
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21
  ; If no need to OR sprites (they never overlap, then we can save a LOT of cycles here!)
#if OR_SPRITE7
  LDD  R19,Y+2             ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE7
  LDD  R19,Y+34            ; Load processed sprite data
  LD   R18,X
  OR   R19,R18
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#else
  LDD  R19,Y+2             ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  LDD  R19,Y+18            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #if V_SPRITE7
  LDD  R19,Y+34            ; Load processed sprite data
  ST   X+,R19              ; Store the result in the line buffer
  #endif
#endif

#endif 

notOnLine7:

#if NUM_SPRITES>8

  ADIW R24,50

  MOVW R28,R24             ; On last sprite, so reg 24:25 no longer used so can use them for collision detection
  MOV  R18,R20             ; move current scanline into R18
  LD   R19,Y               ; sprite Y position
  SUB  R18,R19             ; Check if sprite Y position is below current scanline (scanlineY-spriteY)
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine8          ; 16 or more pixels, so it's not displayed 
  LDD  R19,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into X reg, now 16:17 are no longer used
  ADD  R26,R19             ; Add X position along display line
  //ADC  R27,R21             ; So X is now the destination register ready to accept 3 bytes of sprite data
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21

  LDD  R19,Y+2             ; Load processed sprite data
  LD   R18,X               ; Load byte from current spriteline
  MOV  R16,R19
  OR   R19,R18
  AND  R16,R18             ; Any bits set in R16 indicate a collision
  ST   X+,R19              ; Store the result in the line buffer
  
  LDD  R19,Y+18            ; Load processed sprite data
  LD   R18,X
  MOV  R17,R19
  OR   R19,R18
  AND  R17,R18             ; Any bits set in R17 indicate a collision
  ST   X+,R19              ; Store the result in the line buffer
  
  LDD  R19,Y+34            ; Load processed sprite data
  LD   R18,X
  MOV  R24,R19
  OR   R19,R18
  AND  R24,R18             ; Any bits set in R24 indicate a collision
  ST   X+,R19              ; Store the result in the line buffer

  OR   R24,R16             ; Any bits set in R24 now indicate a collision between sprite 8 and one of the sprites 0-7
  OR   R24,R17
  LDS  R25,collisionBits
  OR   R24,R25
  STS  collisionBits,R24   ; Store in var so C can access

#endif

notOnLine8:

  POP  R16
  POP  R17
  POP  R28
  POP  R29

	RET
