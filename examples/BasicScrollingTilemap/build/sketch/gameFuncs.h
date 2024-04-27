#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\examples\\BasicScrollingTilemap\\gameFuncs.h"
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

#ifndef __GAME_FUNCS
#define __GAME_FUNCS

extern "C" { void storeHi(); }

#include "gameAssets.h"
#include "gameDefs.h"
#include "engine.h"
#include "gameVars.h"

// Declarations

void setState(GameState nextState);
void gameLoop();
void drawText(const uint8_t *txt, uint8_t x, uint8_t y);
void drawText(const uint8_t *charA, uint8_t x, uint8_t y, uint8_t len);


// Implementations

void drawChar(uint8_t chr, uint8_t x, uint8_t y){
  uint8_t *p=screenRam+(y*BYTES_PER_BUFFER_LINE)+x;
  *p=chr;
  *(p+BYTES_PER_BUFFER_LINE)=(chr==0?0:chr+32);
}

void drawText(const uint8_t *charA, uint8_t x, uint8_t y, uint8_t len){
  for(uint8_t n=0;n<len;n++){
    drawChar(pgm_read_byte(charA++),x++,y);
  }
}

void setState(GameState nextState){
  
  switch(nextState){

    case gameInit:
    {
      // Set the background to blank tiles,
      clearTileMap(blankTile);
      setTileRowSplit(16,232);
      uint8_t ix=0;
      for(uint8_t y=0;y<12;y++){
        for(uint8_t x=0;x<16;x++){
          screenRam[((y+9)*32)+(x+1)]=ix++;
        }
      }
      drawText(bottomText,0,2,32);
    }
    break;

    case title:

    break;

    case demo:

    break;

    case pricing:

    break;

    case chooseGameType:

    break;

    case playPlayer:

    break;

    case playing:

    break;

    case levelClear:

    break;

    case gameOver:

    break;

    default:
    break;

  }

  giu=giv=giw=gix=giy=giz=0;
  gameState=nextState;
}

/**
 * @brief Main gameloop runs at 50hz and is synchronised with display
 * 
 */
void gameLoop(){
  // Wait until the current display frame has finished rendering
  waitFrame();

  //drawNumber(0,30,waitFrames,5);
  // Update the state of the controller/button inputs
  readInput();

  // This is the core of the game - we decide what to do based on the current "state" of the game. In this case
  // we are always in the "gameInit" state, and so just animate the sprites and scroll the tiles constantly.
  switch(gameState){
    
    case gameInit:
    {

      // If the X scroll offset has crossed a tile boundary, shift the tilemap across to affect smooth scrolling it
      // is up to the programmer to shift the tiles once they cross an offset boundary, otherwise they'll just "ping" 
      // back again. The Y scroll position is absolute though. There are reasons... erm... it'll be fixed...
      if(giz<100){
        ++giz;
        giu-=1;
        if(giu>127){  // giu is a uint8_t, so when it's under zero, it'll wrap to a value above 127...
          giu+=8;
          shiftTiles(1,blankTile);
        }
      }else{
        ++giz;
        giu+=1;
        if(giu>7){  // giu is a uint8_t, so when it's under zero, it'll wrap to a value above 127...
          giu-=8;
          shiftTiles(-1,blankTile);
        }
        if(giz==200){
          giz=0;
        }
      }

      // "Sway" up and down using a sine wave based on the current frame...
      float r=(PI*2)/256.0;
      gix=190+(int8_t)((sin(r*currentFrame))*50.0);

      setScroll(giu,gix); 


      // Just set the sprite defs up for all 9 sprites based on the current display frame
      for(uint8_t n=0;n<9;n++){

        // Sprite def for this sprite for this frame is based on its index and the current display frame counter
        uint8_t defIX=(n+(currentFrame>>(n>>1)));
        setSpriteDef(n,defIX%8);

        // Just spin the sprites in a circle...
        float r=(PI*2)/200.0;
        uint8_t rad=(giv+(n*5));
        uint8_t sX=116+(int8_t)((sin(r*rad))*116.0);
        uint8_t sY=112+(int8_t)((-sin((PI/2)+(r*rad*3)))*94.0);
        setSpritePos(n,sX,sY);

        if(giv>=200){
          giv-=200;
        }

      }
      ++giv;

      // Scroll text across the top.
      if((currentFrame%5)==0){
        for(uint8_t n=0;n<63;n++){
          screenRam[n]=screenRam[n+1];
        }
        drawChar(pgm_read_byte(scrollText+giw),31,0);
        ++giw;
      }
    }
    break;

    case title:

    break;

    case demo:

    break;

    case pricing:

    break;

    case chooseGameType:

    break;
    
    case playPlayer:

    break;

    case playing:

    break;

    case levelClear:

    break;
        
    case gameOver:

    break;
  }

  // Update the sprite data ready for rendering in the next frame
  processSprites();

  // The current user code will now pause until the display is redrawn
}

#endif