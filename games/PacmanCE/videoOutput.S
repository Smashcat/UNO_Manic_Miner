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
.equ EEARH,  0x22
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
  PUSH R1
  PUSH R0

  MOVW R30,R20;     // Move sprite instance data pointer into Z
  LDI  R20,1
  MOVW R28,R24;     // Move the sprite graphical data pointer into Y
  
  LD R18,Z+         // Sprite Y pos
  ST Y+,R18         // Just copy Y pos into processed sprite data - no need to do anything with it here
  
  LD R18,Z+         // Sprite X pos in R18
  LDS R24,hScroll   // Horizontal scroll is a 16bit value, with lower 3 bits used as bit offset on display
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
  MUL  R19,R22      // Multiply result is in R1:R0 pair
  ADD  R30,R0       // Add the sprite index * 32 to the base pointer address
  ADC  R31,R1
  
  ; //Now loop through the sprite graphical data, dropping it into the processed sprite data, which is 3 x 16 bytes to allow for shifting
  ; // Y is pointing to the destination array
  ; // Z is pointing to the graphical data
  ; // R22 is pointing to the source array offset in program memory
  ; // R18 is the number of bits to shift right
  LDI R19,16        // 16 rows of data

  EOR  R21,R21      // zero out the 3rd byte in the row, so we're not showing old data that's not rotated out!

lineLoop:
  LPM R0,Z+        // Load from program memory
  LPM R1,Z+

  CP R18,R21        // Check number of bits to shift against zero
  BREQ skipShift    // Nothing to shift?

  MOV R24,R18       // Number of bits to shift
  CLC               // Clear the carry flag - no need to do it in loop, as the DEC resets it

shiftLoop:          // Shift sprite graphical data to the right by the horizontal position &0x07 (6 clocks per loop)
  ROR R0
  ROR R1
  ROR R21
  DEC R24
  BRNE shiftLoop
  
skipShift:

  ST Y,R0
  STD Y+16,R1
  STD Y+32,R21

  EOR R21,R21

  ADD R28,R20       // Move Y to point to next destination
  ADC R29,R21
  DEC R19
  BRNE lineLoop;

  POP  R0
  POP  R1
  POP R28
  POP R29
  
  RET










/**
 * This is where the current scanline is rasterised to the display, and also where sprites are rendered to the linebuffer for the next scanline
 */

 .global renderScanline ; (*pScreenRam, *fontSlice)
 ;                          R25  R24     R23 R22

 renderScanline:
	; interrupt may jitter because of instructions taking 1, 2, 3 clock cycles
	; (and maybe more if there are other interrupts)
	; this hack synchronizes the CPU with the tcnt value
	; only checking the low byte here because interrupt should always occur earlier than 240 counts (15 us)
  
  LDS R18,minTCNT
  LDS R20,tcnt

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
  // and also preparing some data for the line-buffer generation after the tiles+previous line-buffer have all been sent to the display. 
  
  EOR  R21,R21                                // +1  R21:R20 = current scanline (R21 is always zero) *

// Byte 0 (doesn't need to fit into 16 clocks, as it's the first, and nothing is sent until the USART is enabled...)
  LD R24, X+ ; get character from screen ram // +2 =  2
  MOVW R30, R22 ; Z = fontSlice              // +1 =  3
  ADD R30, R24                               // +1 =  4
  LPM R20, Z ; fetch 8 pixels                // +3 =  7
  LDS R24,spriteLine ; get lineBuffer byte   // +2 =  9
  OR R20, R24 ; OR the lineBuffer            // +1 = 10
  AND R20,R18 ; Mask off X pixels, based on the current hScroll offset
  STS UDR0, R20 ; send them to the USART     // +2 = 14
  LD R24, X+ ; get character from screen ram // +2 = 19 - had to swap things about to ensure UART kept fed!
  MOVW R30, R22 ; Z = fontSlice              // +1 = 20
  STS spriteLine,R21  ; Clear linebuffer     // +2
  STS UCSR0B, R25 ; enable USART transmit    // +2 = 16 


  // from here we need to keep the USART loaded with new data every 16 clocks - we can load the next byte faster, 
  // but then need to ensure it averages 16 clocks for the byte loads.


// Byte 1
  UNSUPPRESS                                 // +1 = 1
  LDS R18,spriteLine+1 ; get lineBuffer byte // +2 = 3
  CPI R18,0                                  // +1 = 4
  BREQ noSprite1                             // +1 if no jmp, +2 if jmp

  STS spriteLine+1,R21  ; Clear linebuffer   // +2 = 7
  MOV R24,R18                                // +1 = 8
  NOP                                        // +1 = 9
  RJMP uartByte1                             // +2 = 11

noSprite1:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 7
  ADD R30, R24                               // +1 = 8
  LPM R24, Z ; fetch 8 pixels                // +3 = 11

uartByte1:
  PUSH R17                                   // +2 = 13     *
  STS UDR0, R24 ; send them to the USART     // +2 = 15



// Byte 2
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+2 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite2                             // +1 if no jmp, +2 if jmp

  STS spriteLine+2,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte2                             // +2 = 12

noSprite2:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte2:
  PUSH R16                                   // +2 = 14     *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



// Byte 3
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+3 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite3                             // +1 if no jmp, +2 if jmp

  STS spriteLine+3,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte3                             // +2 = 12

noSprite3:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte3:
  LDS  R20,displayLine                       // +2 = 14 R20 now holds the current display line (0 - 8*31) +2                        *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



// Byte 4
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+4 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite4                             // +1 if no jmp, +2 if jmp

  STS spriteLine+4,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte4                             // +2 = 12

noSprite4:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte4:
  MOV  R25,R20                               // *
  INC  R25                                   // *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



// Byte 5
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+5 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite5                             // +1 if no jmp, +2 if jmp

  STS spriteLine+5,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte5                             // +2 = 12

noSprite5:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte5:
  STS  displayLine,R25                       // +2 = 14 Increment the display line and store for the next raster - no NOPs left!!!! *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



// Byte 6
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+6 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite6                             // +1 if no jmp, +2 if jmp

  STS spriteLine+6,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte6                             // +2 = 12

noSprite6:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte6:
  PUSH R29                                   // +2 = 14    *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



// Byte 7
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+7 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite7                             // +1 if no jmp, +2 if jmp

  STS spriteLine+7,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte7                             // +2 = 12

noSprite7:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12
uartByte7:
  PUSH R28                                   // +2 = 14    *
  STS UDR0, R24 ; send them to the USART     // +2 = 16
  


// Byte 8
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+8 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite8                             // +1 if no jmp, +2 if jmp

  STS spriteLine+8,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte8                             // +2 = 12

noSprite8:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte8:
  LDI  R29,hi8(processedSprite+PROCESSED_SPRITE_OFFSET)              // +1 = 13 Load processed sprite data array address into Y register *
  LDI  R28,lo8(processedSprite+PROCESSED_SPRITE_OFFSET)              // +1 = 14                                                          *
  STS UDR0, R24 ; send them to the USART     // +2 = 16

  

// Byte 9
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+9 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite9                             // +1 if no jmp, +2 if jmp

  STS spriteLine+9,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte9                             // +2 = 12

noSprite9:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte9:
  LD   R17,Y               // sprite Y position                                                        +2 *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



// Byte 10
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+10 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite10                             // +1 if no jmp, +2 if jmp

  STS spriteLine+10,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte10                             // +2 = 12

noSprite10:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte10:
  MOV  R16,R20             // move current scanline into R16                                           +1 *
  SUB  R16,R17             // Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



// Byte 11
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+11 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite11                             // +1 if no jmp, +2 if jmp

  STS spriteLine+11,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte11                             // +2 = 12

noSprite11:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte11:
  SBIW R28,50                                   // Move back through processed Sprite array (200)                                +2 *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



  // Byte 12
  LD R24, X+ ; get character from screen ram  // +2 = 2
  LDS R18,spriteLine+12 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  BREQ noSprite12                             // +1 if no jmp, +2 if jmp

  STS spriteLine+12,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                 // +1 = 9
  NOP                                         // +1 = 10
  RJMP uartByte12                             // +2 = 12

noSprite12:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R24                                // +1 = 9
  LPM R24, Z ; fetch 8 pixels                 // +3 = 12

uartByte12:
  OUT EEDR,R16                                // +1 *
  MOV  R16,R20                                // move current scanline into R16 +1 *
  STS UDR0, R24 ; send them to the USART      // +2 = 16



  // Byte 13
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+13 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite13                             // +1 if no jmp, +2 if jmp

  STS spriteLine+13,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte13                             // +2 = 12

noSprite13:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte13:
  LD   R17,Y               // sprite Y position                                                        +2 *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



  // Byte 14
  LD R24, X+ ; get character from screen ram // +2 = 2
  LDS R18,spriteLine+14 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                  // +1 = 5
  BREQ noSprite14                             // +1 if no jmp, +2 if jmp

  STS spriteLine+14,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                // +1 = 9
  NOP                                        // +1 = 10
  RJMP uartByte14                             // +2 = 12

noSprite14:
  MOVW R30, R22 ; Z = fontSlice              // +1 = 8
  ADD R30, R24                               // +1 = 9
  LPM R24, Z ; fetch 8 pixels                // +3 = 12

uartByte14:
  SUB  R16,R17             // Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
  OUT EEARL,R16            //                                                                          +1 *
  STS UDR0, R24 ; send them to the USART     // +2 = 16



  // Byte 15
  LD R24, X+ ; get character from screen ram  // +2 = 2
  LDS R18,spriteLine+15 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  BREQ noSprite15                             // +1 if no jmp, +2 if jmp

  STS spriteLine+15,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                 // +1 = 9
  NOP                                         // +1 = 10
  RJMP uartByte15                             // +2 = 12

noSprite15:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R24                                // +1 = 9
  LPM R24, Z ; fetch 8 pixels                 // +3 = 12

uartByte15:
  SBIW R28,50              // Move back through processed Sprite array (150)                                 +2 *
  STS UDR0, R24 ; send them to the USART      // +2 = 16




  // Byte 16
  LD R24, X+ ; get character from screen ram  // +2 = 2
  LDS R18,spriteLine+16 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  BREQ noSprite16                             // +1 if no jmp, +2 if jmp

  STS spriteLine+16,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                 // +1 = 9
  NOP                                         // +1 = 10
  RJMP uartByte16                             // +2 = 12

noSprite16:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R24                                // +1 = 9
  LPM R24, Z ; fetch 8 pixels                 // +3 = 12

uartByte16:
  LD   R17,Y               // sprite Y position                                                        +2 *
  STS UDR0, R24 ; send them to the USART      // +2 = 16



  // Byte 17
  LD R24, X+ ; get character from screen ram  // +2 = 2
  LDS R18,spriteLine+17 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  BREQ noSprite17                             // +1 if no jmp, +2 if jmp

  STS spriteLine+17,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                 // +1 = 9
  NOP                                         // +1 = 10
  RJMP uartByte17                             // +2 = 12

noSprite17:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R24                                // +1 = 9
  LPM R24, Z ; fetch 8 pixels                 // +3 = 12

uartByte17:
  MOV  R16,R20             // move current scanline into R16                                           +1 *
  SUB  R16,R17             // Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
  STS UDR0, R24 ; send them to the USART      // +2 = 16
  


// Byte 18
  LD R24, X+ ; get character from screen ram  // +2 = 2
  LDS R18,spriteLine+18 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  BREQ noSprite18                             // +1 if no jmp, +2 if jmp

  STS spriteLine+18,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                 // +1 = 9
  NOP                                         // +1 = 10
  RJMP uartByte18                             // +2 = 12

noSprite18:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R24                                // +1 = 9
  LPM R24, Z ; fetch 8 pixels                 // +3 = 12

uartByte18:
  SBIW R28,50              // Move back through processed Sprite array    (100)                             +2 *
  STS UDR0, R24 ; send them to the USART      // +2 = 16



// Byte 19
  LD R24, X+ ; get character from screen ram  // +2 = 2
  LDS R18,spriteLine+19 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  BREQ noSprite19                             // +1 if no jmp, +2 if jmp

  STS spriteLine+19,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                 // +1 = 9
  NOP                                         // +1 = 10
  RJMP uartByte19                             // +2 = 12

noSprite19:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R24                                // +1 = 9
  LPM R24, Z ; fetch 8 pixels                 // +3 = 12

uartByte19:
  OUT GPIOR2,R16            //                                                                          +1 *
  MOV  R16,R20             // move current scanline into R16                                           +1 *
  STS UDR0, R24 ; send them to the USART      // +2 = 16



// Byte 20
  LD R24, X+ ; get character from screen ram  // +2 = 2
  LDS R18,spriteLine+20 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  BREQ noSprite20                             // +1 if no jmp, +2 if jmp

  STS spriteLine+20,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                 // +1 = 9
  NOP                                         // +1 = 10
  RJMP uartByte20                             // +2 = 12

noSprite20:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R24                                // +1 = 9
  LPM R24, Z ; fetch 8 pixels                 // +3 = 12

uartByte20:
  LD   R17,Y               // sprite Y position                                                        +2 *
  STS UDR0, R24 ; send them to the USART      // +2 = 16



// Byte 21
  LD R24, X+ ; get character from screen ram  // +2 = 2
  LDS R18,spriteLine+21 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  BREQ noSprite21                             // +1 if no jmp, +2 if jmp

  STS spriteLine+21,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                 // +1 = 9
  NOP                                         // +1 = 10
  RJMP uartByte21                             // +2 = 12

noSprite21:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R24                                // +1 = 9
  LPM R24, Z ; fetch 8 pixels                 // +3 = 12

uartByte21:
  SUB  R16,R17             // Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
  OUT GPIOR1,R16            //                                                                          +1 *
  STS UDR0, R24 ; send them to the USART      // +2 = 16



// Byte 22
  LD R24, X+ ; get character from screen ram  // +2 = 2
  LDS R18,spriteLine+22 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  BREQ noSprite22                             // +1 if no jmp, +2 if jmp

  STS spriteLine+22,R21  ; Clear linebuffer   // +2 = 8
  MOV R24,R18                                 // +1 = 9
  NOP                                         // +1 = 10
  RJMP uartByte22                             // +2 = 12

noSprite22:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R24                                // +1 = 9
  LPM R24, Z ; fetch 8 pixels                 // +3 = 12

uartByte22:
  SBIW R28,50              // Move back through processed Sprite array  (50)                               +2 *
  STS UDR0, R24 ; send them to the USART      // +2 = 16



// Byte 23
  LDS R18,spriteLine+23 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  LD R18, X+ ; get character from screen ram  // +2 = 2
  BREQ noSprite23                             // +1 if no jmp, +2 if jmp

  LDS R18,spriteLine+23 ; get lineBuffer byte // +2 = 4
  STS spriteLine+23,R21  ; Clear linebuffer   // +2 = 8
  RJMP uartByte23                             // +2 = 12

noSprite23:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R18                                // +1 = 9
  LPM R18, Z ; fetch 8 pixels                 // +3 = 12

uartByte23:
  LD   R17,Y               // sprite Y position                                                        +2 *
  STS UDR0, R18 ; send them to the USART      // +2 = 16



// Byte 24
  LDS R18,spriteLine+24 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  LD R18, X+ ; get character from screen ram  // +2 = 2
  BREQ noSprite24                             // +1 if no jmp, +2 if jmp

  LDS R18,spriteLine+24 ; get lineBuffer byte // +2 = 4
  STS spriteLine+24,R21  ; Clear linebuffer   // +2 = 8
  RJMP uartByte24                             // +2 = 12

noSprite24:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R18                                // +1 = 9
  LPM R18, Z ; fetch 8 pixels                 // +3 = 12

uartByte24:
  MOV  R24,R20             // move current scanline into R16                                           +1 *
  SUB  R24,R17             // Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
  STS UDR0, R18 ; send them to the USART      // +2 = 16



// Byte 25
  LDS R18,spriteLine+25 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  LD R18, X+ ; get character from screen ram  // +2 = 2
  BREQ noSprite25                             // +1 if no jmp, +2 if jmp

  LDS R18,spriteLine+25 ; get lineBuffer byte // +2 = 4
  STS spriteLine+25,R21  ; Clear linebuffer   // +2 = 8
  RJMP uartByte25                             // +2 = 12

noSprite25:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R18                                // +1 = 9
  LPM R18, Z ; fetch 8 pixels                 // +3 = 12

uartByte25:
  SBIW R28,50              // Move back through processed Sprite array  (0)                            +2 *
  STS UDR0, R18 ; send them to the USART      // +2 = 16



// Byte 26
  LDS R18,spriteLine+26 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  LD R18, X+ ; get character from screen ram  // +2 = 2
  BREQ noSprite26                             // +1 if no jmp, +2 if jmp

  LDS R18,spriteLine+26 ; get lineBuffer byte // +2 = 4
  STS spriteLine+26,R21  ; Clear linebuffer   // +2 = 8
  RJMP uartByte26                             // +2 = 12

noSprite26:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R18                                // +1 = 9
  LPM R18, Z ; fetch 8 pixels                 // +3 = 12

uartByte26:
  LD   R17,Y               // sprite Y position                                                        +2 *
  STS UDR0, R18 ; send them to the USART      // +2 = 16



// Byte 27
  LDS R18,spriteLine+27 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  LD R18, X+ ; get character from screen ram  // +2 = 2
  BREQ noSprite27                             // +1 if no jmp, +2 if jmp

  LDS R18,spriteLine+27 ; get lineBuffer byte // +2 = 4
  STS spriteLine+27,R21  ; Clear linebuffer   // +2 = 8
  RJMP uartByte27                             // +2 = 12

noSprite27:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R18                                // +1 = 9
  LPM R18, Z ; fetch 8 pixels                 // +3 = 12

uartByte27:
  MOV  R19,R20              // move current scanline into R19                                           +1 *
  SUB  R19,R17             // Check if sprite Y position is below current scanline (scanlineY-spriteY) +1 *
  STS UDR0, R18 ; send them to the USART      // +2 = 16



// Byte 28
  LDS R18,spriteLine+28 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  LD R18, X+ ; get character from screen ram  // +2 = 2
  BREQ noSprite28                             // +1 if no jmp, +2 if jmp

  LDS R18,spriteLine+28 ; get lineBuffer byte // +2 = 4
  STS spriteLine+28,R21  ; Clear linebuffer   // +2 = 8
  RJMP uartByte28                             // +2 = 12

noSprite28:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R18                                // +1 = 9
  LPM R18, Z ; fetch 8 pixels                 // +3 = 12

uartByte28:
  NOP
  NOP
  STS UDR0, R18 ; send them to the USART      // +2 = 16



// Byte 29
  LDS R18,spriteLine+29 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  LD R18, X+ ; get character from screen ram  // +2 = 2
  BREQ noSprite29                             // +1 if no jmp, +2 if jmp

  LDS R18,spriteLine+29 ; get lineBuffer byte // +2 = 4
  STS spriteLine+29,R21  ; Clear linebuffer   // +2 = 8
  RJMP uartByte29                             // +2 = 12

noSprite29:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R18                                // +1 = 9
  LPM R18, Z ; fetch 8 pixels                 // +3 = 12

uartByte29:
  LDI  R17,hi8(spriteLine)                    // +1 = 13 Linebuffer address into R17:R16 register pair    *
  LDI  R16,lo8(spriteLine)                    // +1 = 14                                                  *
  STS UDR0, R18 ; send them to the USART      // +2 = 16



 // Byte 30
  LDS R18,spriteLine+30 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  LD R18, X+ ; get character from screen ram  // +2 = 2
  BREQ noSprite30                             // +1 if no jmp, +2 if jmp

  LDS R18,spriteLine+30 ; get lineBuffer byte // +2 = 4
  STS spriteLine+30,R21  ; Clear linebuffer   // +2 = 8
  RJMP uartByte30                             // +2 = 12

noSprite30:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R18                                // +1 = 9
  LPM R18, Z ; fetch 8 pixels                 // +3 = 12

uartByte30:
  LDS R25,hScrollEMask;                       // +2 = 14
  STS UDR0, R18 ; send them to the USART      // +2 = 16



// Byte 31
  LDS R18,spriteLine+31 ; get lineBuffer byte // +2 = 4
  CPI R18,0                                   // +1 = 5
  LD  R18, X+ ; get character from screen ram // +2 = 2
  BREQ noSprite31                             // +1 if no jmp, +2 if jmp

  LDS R18,spriteLine+31 ; get lineBuffer byte // +2 = 4
  STS spriteLine+31,R21  ; Clear linebuffer   // +2 = 8
  RJMP uartByte31                             // +2 = 12

noSprite31:
  MOVW R30, R22 ; Z = fontSlice               // +1 = 8
  ADD R30, R18                                // +1 = 9
  LPM R18, Z ; fetch 8 pixels                 // +3 = 12

uartByte31:
  AND R18,R25                                 // +1 = 13 AND the end of line mask
  MOVW R30,R28                                // +1 = 14 Store pointer to start of processedSprite data to reset Y for each sprite
  STS UDR0, R18 ; send them to the USART      // +2 = 16

  STS UCSR0B, R1 ; disable USART transmit
  










;===========================================
; Now get sprites ready for next scanline...
;===========================================

  ; 22,23,25 = Free
  ; 18    = holds the data row from the sprite to display (0-15) after initial check
  ; 19    = Y offset for first sprite on entry
  ; 24    = Y offset for second sprite on entry
  ; 17:16 = addr of displayLine
  ; 21:20 = current scanline (R21 is always zero here)
  ; 27:26 = points to displayLine to write line buffer (X)
  ; 29:28 = pointer to processedSprite data (Y)
  ; 31:30 = Holds pointer to NEXT element in processedSprite data array

#if NUM_SPRITES>0

  CPI  R19,16              ; R19 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine0          ; 16 or more pixels, so it's not displayed 
  
  LDD  R22,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into X
  ADD  R26,R22             ; Add X position along display line
  ADD  R28,R19             ; Y now points to source position from processed sprite data  
  ADC  R29,R21

  LDD  R22,Y+2             ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+18            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+34            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer

#endif

notOnLine0:

#if NUM_SPRITES>1

  ADIW R30,50

  CPI  R24,16              ; R19 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine1          ; 16 or more pixels, so it's not displayed 
 
  MOVW R28,R30
  LDD  R22,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R22             ; Add X position along display line
  ADD  R28,R24             ; Y now points to source position from processed sprite data  
  ADC  R29,R21

  LDD  R22,Y+2             ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+18            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+34            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer

#endif

notOnLine1:

#if NUM_SPRITES>2

  ADIW R30,50

  IN R18,GPIOR1
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine2          ; 16 or more pixels, so it's not displayed 

  MOVW R28,R30
  LDD  R22,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R22             ; Add X position along display line
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21

  LDD  R22,Y+2             ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+18            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+34            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer

#endif

notOnLine2:

#if NUM_SPRITES>3

  ADIW R30,50

  IN R18,GPIOR2 
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine3          ; 16 or more pixels, so it's not displayed 

  MOVW R28,R30
  LDD  R22,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R22             ; Add X position along display line
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21

  LDD  R22,Y+2             ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+18            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+34            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer

#endif

notOnLine3:

#if NUM_SPRITES>4

  ADIW R30,50

  IN R18,EEARL 
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine4          ; 16 or more pixels, so it's not displayed 
  
  MOVW R28,R30
  LDD  R22,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R22             ; Add X position along display line
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21

  LDD  R22,Y+2             ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+18            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+34            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer

#endif

notOnLine4:

#if NUM_SPRITES>5

  ADIW R30,50

  IN R18,EEDR 
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine5          ; 16 or more pixels, so it's not displayed 

  MOVW R28,R30
  LDD  R22,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R22             ; Add X position along display line
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21

  LDD  R22,Y+2             ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+18            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+34            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer

#endif

notOnLine5:

#if NUM_SPRITES>6

  ADIW R30,50

  MOVW R28,R30
  MOV  R18,R20             ; move current scanline into R18
  LD   R22,Y               ; sprite Y position
  SUB  R18,R22             ; Check if sprite Y position is below current scanline (scanlineY-spriteY)
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine6          ; 16 or more pixels, so it's not displayed 
  
  LDD  R22,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into Z
  ADD  R26,R22             ; Add X position along display line
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21

  LDD  R22,Y+2             ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+18            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+34            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer

#endif 

notOnLine6:

#if NUM_SPRITES>7

  ADIW R30,50

  MOVW R28,R30
  MOV  R18,R20             ; move current scanline into R18
  LD   R22,Y               ; sprite Y position
  SUB  R18,R22             ; Check if sprite Y position is below current scanline (scanlineY-spriteY)
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine7          ; 16 or more pixels, so it's not displayed 

  LDD  R22,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into X
  ADD  R26,R22             ; Add X position along display line
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21             ; Total of 14 clocks 

  LDD  R22,Y+2             ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+18            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+34            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
                           ; Total of 12 clocks, best case if not a vertical-only sprite

#endif 

notOnLine7:

#if NUM_SPRITES>8

  ADIW R30,50

  MOVW R28,R30             ; Load processed sprite data into Y
  MOV  R18,R20             ; move current scanline into R18
  LD   R22,Y               ; sprite Y position
  SUB  R18,R22             ; Check if sprite Y position is below current scanline (scanlineY-spriteY)
  CPI  R18,16              ; R18 is positive, so check if 16 pixels or less above scanline
  BRCC notOnLine8          ; 16 or more pixels, so it's not displayed 

  LDD  R22,Y+1             ; sprite X position
  MOVW R26,R16             ; Load display line address into X reg
  ADD  R26,R22             ; Add X position along display line
  ADD  R28,R18             ; Y now points to source position from processed sprite data  
  ADC  R29,R21

  // No longer do collision detection on sprite 8, as we can do it at any time outside of the renderer
  LDD  R22,Y+2             ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+18            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer
  LDD  R22,Y+34            ; Load processed sprite data
  ST   X+,R22              ; Store the result in the line buffer

#endif

notOnLine8:

  //if (tcnt < minTCNT) - jitter adjust
  //      minTCNT = tcnt;
  LDS  R18,minTCNT
  LDS  R20,tcnt
  CP   R20,R18
  BRGE noTcntChange
  STS minTCNT,R20

noTcntChange:
  POP R28
  POP R29
  POP R16
  POP R17

	RET
