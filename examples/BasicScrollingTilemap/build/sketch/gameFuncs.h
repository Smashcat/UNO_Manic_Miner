#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\examples\\BasicScrollingTilemap\\gameFuncs.h"
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

    // Just set the sprite defs up for all 9 sprites based on the current display frame
    for(uint8_t n=0;n<9;n++){
      // Sprite def for this sprite for this frame is based on its index and the current display frame counter
      uint8_t defIX=(n+(currentFrame>>(n>>1)));
      // Update sprites
      setSpriteDef(n,defIX%4);
      // They never actually change position here, but could pretty easily...
      setSpritePos(n,80+((n%3)*32),80+(n/3)*32);
    }

    // Scroll up by 1 pixel per frame
    setScroll(0,currentFrame); 

    // If we've scrolled to a tile boundary, then clear and add an invader at the bottom of the tilemap (relative to the scroll position)
    // Note: The BOTTOM_EDGE setting in the engineDef.h file defines how many of the tilemap rows we render. We have 32 rows but do not
    // rander the bottom row, as it allows us to draw to it while it is hidden, allowing tiles to appear smoothly as they scroll into view.
    if((currentFrame&0x0f)==0){

      // Find the bottom-most tile row, relative to the current Y scroll position
      uint8_t bottomRow=((currentFrame>>3)+31)%32;

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