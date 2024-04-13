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


// Implementations

void setState(GameState nextState){
  
  switch(nextState){

    case gameInit:
      // Set the background to blank tiles,
      clearTileMap(blankTile);
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
      // Just set the sprite defs up for all 9 sprites based on the current display frame
      for(uint8_t n=0;n<9;n++){
        // Sprite def for this sprite for this frame is based on its index and the current display frame counter
        uint8_t defIX=(n+(currentFrame>>(n>>1)));
        // Update sprites
        setSpriteDef(n,defIX%4);
        // They never actually change position here, but could pretty easily...
        setSpritePos(n,80+((n%3)*32),80+(n/3)*32);
      }

      // currentFrame increments each frame, and rolls over after 255 back to 0. So gix will count from 255-0, meaning we'll scroll to the right
      gix=0-(currentFrame<<1);

      // If the X scroll offset has crossed a tile boundary, shift the tilemap across to affect smooth scrolling
      giu-=3;
      if(giu>127){
        giu+=8;
        shiftTiles(1,fontIX+((currentFrame>>3)&0x07));
      }

      // Scroll down by 2 pixels per frame and across by 1 pixel. Note that the X scroll position is an offset only. This is between 0 and 7, it
      // is up to the programmer to shift the tiles once they cross an offset boundary, otherwise they'll just "ping" back again. The Y scroll
      // position is absolute though. There are reasons... erm... it'll be fixed...
      setScroll(giu,gix); 

      // If we've scrolled to a tile boundary, then clear and add an invader at the bottom of the tilemap (relative to the scroll position)
      // Note: The BOTTOM_EDGE setting in the engineDef.h file defines how many of the tilemap rows we render. We have 32 rows but do not
      // rander the bottom row, as it allows us to draw to it while it is hidden, allowing tiles to appear smoothly as they scroll into view.
      if((gix&0x07)==0){

        // Find the bottom-most tile row, relative to the current Y scroll position
        uint8_t bottomRow=((gix>>3)+31)%32;

        // If on a 16 pixel vertical boundary, draw a random invader and clear a random invader position
        if((gix&0x0f)==0){

          // Clear a random tile pair
          uint8_t xOff=random(16)<<1;
          screenRam[(BYTES_PER_BUFFER_LINE*bottomRow)+xOff]=blankTile;
          screenRam[(BYTES_PER_BUFFER_LINE*bottomRow)+xOff+1]=blankTile;

          // Draw a random invader at a tile pair
          xOff=random(16)<<1;
          uint8_t invaderType=225+(random(4)*2);
          screenRam[(BYTES_PER_BUFFER_LINE*bottomRow)+xOff]=invaderType;
          screenRam[(BYTES_PER_BUFFER_LINE*bottomRow)+xOff+1]=invaderType+1;
        }

        // Draw blocks on the left edge of the row
        screenRam[(BYTES_PER_BUFFER_LINE*bottomRow)+0]=255;

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