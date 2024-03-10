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

volatile uint16_t scanline = 0; // counts 0 - 311 (312 scan lines per frame)
volatile uint16_t minTCNT = 0xFFFF;
volatile uint16_t tcnt;
volatile const uint8_t *fontSlice;
volatile uint8_t tileDataIX=0;
volatile uint8_t currentFrame=0;
volatile uint8_t displayLine=0;
volatile uint8_t collisionBits=0;
volatile uint8_t *pScreenRam;

uint8_t screenRam[SCREEN_RAM_SIZE];

uint16_t hScroll=0;
uint16_t vScroll=8*41;
uint8_t tileRowSplit=40;
uint8_t tileRowStart=0;
uint16_t lineCompare=600;
int8_t scrollDir=1;
int8_t hScrollDir=1;
uint8_t hScrollSMask=0xff;
uint8_t hScrollEMask=0xff;
const uint8_t *spriteDefRef;  // ASM cannot get a pointer directly to the const array - it also gives the possibility of bank switching sprites

uint8_t lastFrame=0;
int8_t spriteDir[NUM_SPRITES]={1,2,3,2,1,2,3,2,1};
uint8_t spriteMax[NUM_SPRITES]={255,255,255,255,255,255,255,255,255};
uint8_t spriteMin[NUM_SPRITES]={0,0,0,0,0,0,0,0,0};
uint8_t spriteFrame[NUM_SPRITES]={0,0,0,0,0,0,0,0,0};
uint16_t scrollTmr=128;

uint8_t musicTmr=2;
uint8_t resetMusicTmr=2;
uint8_t musicFF=0;
uint8_t musicPos=0;
uint8_t level=0;
uint8_t levelLoadCnt=0;
uint8_t vScrollSub=0;
uint8_t spriteFrameIX[8];
uint8_t willyFlags[2];
uint8_t verticalGuardian;
uint8_t eugeneGuardian;
uint8_t kongGuardian;
uint8_t levelFlags;
uint8_t scrollBuff[4];
uint8_t lives=0;
uint8_t air=0;
uint8_t gameFlags=0;
uint8_t gameFlags2=GF2_ENABLE_MUSIC;
int8_t  xVel,yVel,vXVel;
uint8_t jmpV=0;
uint8_t sfxV=0;
uint8_t fallDist=0;
uint16_t pickup[5];
uint16_t score=0;
uint16_t hiScore=0;
uint8_t altFontLine=255;
uint8_t restoreFontLine=0;
uint8_t altFontIX=1;
uint8_t sunFrame=0;
uint8_t cursorY=0;
uint8_t startLevel=START_LEVEL;

KeyState keyState;
LevelState levelState=levelNotLoaded;
AudioType audioType=muted;
GameState gameState=idle;
uint8_t giu,giv,giw,gix,giy,giz;  // General purpose registers for game

volatile uint8_t spriteLine[BYTES_PER_RASTER+2]__attribute__((aligned(32))) ={0};

// y,x,defIX
SpriteData spriteData[NUM_SPRITES]={
  {2,80,0},
  {6,150,4},
  {8,15,8},
  {8,8,12},
  {2,17,16},
  {4,22,20},
  {28,19,24},
  {46,26,28},
  {0,0,0}
};

ProcessedSprite processedSprite[NUM_SPRITES];
