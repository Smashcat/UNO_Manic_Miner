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

// Pac-Man Championship (timed mode). The world is a left half-map, a fixed centre strip and a right half-map
// (47x23 cells, one 8x8 tile each) that wraps around in both directions. The 32x24 tile screen scrolls horizontally
// over it by shifting the tilemap one tile every 8 pixels and filling in the new column. The maze is only 184
// pixels tall so there is no vertical scrolling - going off the top or bottom just wraps to the other side.
//
// The same scrolling code also runs the animated band on the title screen and slides the high score page on and off:
// "scrollMode" selects where the tiles for the scrolling part of the screen come from.

#ifndef __GAME_FUNCS
#define __GAME_FUNCS

#include "gameAssets.h"
#include "gameDefs.h"
#include "engine.h"
#include "gameVars.h"
#include "audio.h"
#include "scores.h"

// Declarations

void setState(GameState nextState);
void gameLoop();
void drawChar(const uint8_t chr, uint8_t x, uint8_t y);
void drawStr_P(const char *s, uint8_t x, uint8_t y);
void drawNumPad(uint32_t num, uint8_t x, uint8_t y, uint8_t digits);
void clearBlock(uint8_t startX, uint8_t startY, uint8_t len);
void startNextWipe();

// Implementations

// (small tables live in flash - a plain const array would be copied into the precious RAM)
const int8_t dxTab[4] PROGMEM={0,1,0,-1};
const int8_t dyTab[4] PROGMEM={-1,0,1,0};
#define DX(d)  ((int8_t)pgm_read_byte(&dxTab[d]))
#define DY(d)  ((int8_t)pgm_read_byte(&dyTab[d]))
// Sprite frame for each direction (the frames in the asset file are ordered up, down, left, right)
const uint8_t pacBaseFrame[4] PROGMEM={SPR_PAC_UP_0,SPR_PAC_RIGHT_0,SPR_PAC_DOWN_0,SPR_PAC_LEFT_0};
// Points for each icon, in the order the game works through them (the icons are in that order in the assets too)
const uint16_t fruitScore[NUM_ICONS] PROGMEM={
  1000,1200,1400,1600,1800,2000,2200,2400,   // cherry, strawberry, orange, apple, watermelon, banana, peach, Galaxian
  2600,4000,4200,4400,4600,4800,5000,5200,   // bell, key, coffee, cake, Galaga flagship, space bug, burger, scrambled egg
  5400,5600,5800,6000,6200,7650              // candy, four-leaf clover, diamond, heart, bat with gold wings, crown
};

// ---- Wrapping helpers --------------------------------------------------------------------------------------

int16_t wrapTo(int16_t v, int16_t m){
  if(v<0){ v+=m; }
  else if(v>=m){ v-=m; }
  return v;
}

/// @brief Shortest signed distance on a ring of size m
int16_t wrapDelta(int16_t d, int16_t m){
  d%=m;
  if(d>(m/2)){ d-=m; }
  else if(d<-(m/2)){ d+=m; }
  return d;
}

// ---- Text and HUD --------------------------------------------------------------------------------------------

void drawChar(const uint8_t chr, uint8_t x, uint8_t y){
  screenRam[(y*BYTES_PER_BUFFER_LINE)+x]=chr;
}

uint8_t charTile(char c){
  if(c>='A' && c<='Z') return TILE_A+(c-'A');
  if(c>='0' && c<='9') return c-'0';
  if(c==':') return TILE_COLON;
  if(c=='-') return TILE_DASH;
  return BLANK_TILE;
}

void drawStr_P(const char *s, uint8_t x, uint8_t y){
  char c;
  while((c=pgm_read_byte(s++))){
    drawChar(charTile(c),x++,y);
  }
}

void drawNumPad(uint32_t num, uint8_t x, uint8_t y, uint8_t digits){
  for(uint8_t i=digits;i>0;i--){
    drawChar(num%10,x+i-1,y);
    num/=10;
  }
}

void clearBlock(uint8_t startX, uint8_t startY, uint8_t len){
  uint8_t *p=screenRam+(startY*BYTES_PER_BUFFER_LINE)+startX;
  while(len--){
    *p++=BLANK_TILE;
  }
}

// Double height text for the HUD: each character is a top tile and a bottom tile
uint8_t bigIndex(char c){
  if(c>='0' && c<='9') return c-'0';
  if(c>='A' && c<='Z') return 10+(c-'A');
  if(c==':') return 36;
  return 255;
}

void drawBigChar(char c, uint8_t x, uint8_t y){
  uint8_t i=bigIndex(c);
  if(i==255){
    drawChar(BLANK_TILE,x,y);
    drawChar(BLANK_TILE,x,y+1);
    return;
  }
  drawChar(pgm_read_byte(&bigFontTop[i]),x,y);
  drawChar(pgm_read_byte(&bigFontBot[i]),x,y+1);
}

void drawBigStr_P(const char *s, uint8_t x, uint8_t y){
  char c;
  while((c=pgm_read_byte(s++))){
    drawBigChar(c,x++,y);
  }
}

void drawBigNum(uint32_t num, uint8_t x, uint8_t y, uint8_t digits){
  for(uint8_t i=digits;i>0;i--){
    drawBigChar('0'+(num%10),x+i-1,y);
    num/=10;
  }
}

void drawHud(){
  drawBigStr_P(PSTR("SCORE"),1,1);
  drawBigNum(score,7,1,6);
  // The time finishes at the right hand edge of the screen
  drawBigStr_P(PSTR("TIME"),23,1);
  uint16_t secs=(framesLeft+49)/50;
  drawBigNum(secs/60,28,1,1);
  drawBigChar(':',29,1);
  drawBigNum(secs%60,30,1,2);
  // Lives are 16x16 Pac-Men
  clearBlock(1,LIVES_ROW,8);
  clearBlock(1,LIVES_ROW+1,8);
  for(uint8_t i=0;i<lives;i++){
    drawChar(pgm_read_byte(&lifeTiles[0]),1+(i*2),LIVES_ROW);
    drawChar(pgm_read_byte(&lifeTiles[1]),2+(i*2),LIVES_ROW);
    drawChar(pgm_read_byte(&lifeTiles[2]),1+(i*2),LIVES_ROW+1);
    drawChar(pgm_read_byte(&lifeTiles[3]),2+(i*2),LIVES_ROW+1);
  }
  if(demoMode){
    drawBigStr_P(PSTR("DEMO"),1,LIVES_ROW);
  }
  hudDirty=0;
}

// ---- Misc helpers --------------------------------------------------------------------------------------------

uint8_t rand8(){
  rng^=rng<<7;
  rng^=rng>>9;
  rng^=rng<<8;
  return (uint8_t)rng;
}

// ---- World -----------------------------------------------------------------------------------------------------

/// @brief Kind of a cell in a map. Maps are packed two cells to a byte
uint8_t mapKind(uint8_t mapIx, uint16_t ix){
  uint8_t b=pgm_read_byte(&mapData[mapIx][ix>>1]);
  return (ix&1)?(b>>4):(b&15);
}

/// @brief Which map is used for column lx of a half. While a half is being replaced, columns the wave has already
/// passed use the new map and the rest still use the old one
uint8_t mapFor(uint8_t half, uint8_t lx){
  if(wipeHalf==half){
    uint8_t dist=(half==0)?(MAP_W-1-lx):lx;
    if(dist<wipeProg){
      return wipeNewMap;
    }
  }
  return halfMap[half];
}

/// @brief World column for a given distance from the centre, in the half being replaced
uint8_t wipeColumn(uint8_t dist){
  return (wipeHalf==0)?(MAP_W-1-dist):(RIGHT_HALF_X+dist);
}

/// @brief How far from the centre a column is, if it is in the half being replaced (otherwise 255)
uint8_t wipeDist(uint8_t cx){
  if(wipeHalf==0){
    return (cx<MAP_W)?(MAP_W-1-cx):255;
  }
  if(wipeHalf==1){
    return (cx>=RIGHT_HALF_X)?(cx-RIGHT_HALF_X):255;
  }
  return 255;
}

/// @brief Kind of the cell at a world cell position (coordinates must already be wrapped), taking into account
/// dots that have been eaten
uint8_t cellKind(uint8_t cx, uint8_t cy){
  uint8_t half;
  uint8_t lx;
  if(cx<MAP_W){
    half=0;
    lx=cx;
  }else if(cx<RIGHT_HALF_X){
    return pgm_read_byte(&centerData[(cy*CENTER_W)+(cx-MAP_W)]);
  }else{
    half=1;
    lx=cx-RIGHT_HALF_X;
  }
  uint16_t ix=((uint16_t)cy*MAP_W)+lx;
  uint8_t k=mapKind(mapFor(half,lx),ix);
  if((k==K_DOT || k==K_PELLET) && (dotBits[half][ix>>3]&(1<<(ix&7)))==0){
    k=K_PATH;
  }
  return k;
}

bool walkable(int8_t cx, int8_t cy){
  uint8_t k=cellKind(wrapTo(cx,WORLD_W),wrapTo(cy,WORLD_H));
  return k>=K_PATH && k<=K_PELLET;
}

// ---- Score trail -----------------------------------------------------------------------------------------------
// Dots that Pac-Man eats turn into the number of points they were worth, which then disappears a little later

void redrawCell(uint8_t cx, uint8_t cy);

/// @brief Points/10 shown at this cell, or 0 if there is no trail dot there
uint8_t trailValueAt(uint8_t cx, uint8_t cy){
  for(uint8_t i=0;i<TRAIL_SLOTS;i++){
    if(trail[i].timer && trail[i].cx==cx && trail[i].cy==cy){
      return trail[i].value;
    }
  }
  return 0;
}

void trailAdd(uint8_t cx, uint8_t cy, uint8_t value){
  // Use a free slot, or replace the one that is nearest to disappearing
  uint8_t slot=0;
  for(uint8_t i=0;i<TRAIL_SLOTS;i++){
    if(trail[i].timer==0){
      slot=i;
      break;
    }
    if(trail[i].timer<trail[slot].timer){
      slot=i;
    }
  }
  if(trail[slot].timer){
    uint8_t ocx=trail[slot].cx,ocy=trail[slot].cy;
    trail[slot].timer=0;
    redrawCell(ocx,ocy);
  }
  trail[slot].cx=cx;
  trail[slot].cy=cy;
  trail[slot].value=value;
  trail[slot].timer=TRAIL_FRAMES;
}

void trailClear(){
  for(uint8_t i=0;i<TRAIL_SLOTS;i++){
    trail[i].timer=0;
  }
}

void trailUpdate(){
  for(uint8_t i=0;i<TRAIL_SLOTS;i++){
    if(trail[i].timer && --trail[i].timer==0){
      redrawCell(trail[i].cx,trail[i].cy);
    }
  }
}

/// @brief The tile to show for a world cell. While a half-map is being replaced, a column of Pac-Man tiles sweeps
/// out from the centre; behind it the maze is shown "filled in", and a ripple then turns it back to normal
uint8_t cellTileAt(uint8_t cx, uint8_t cy){
  uint8_t tv=trailValueAt(cx,cy);
  if(tv){
    return pgm_read_byte(&trailTiles[tv-1]);
  }
  uint8_t kind=cellKind(cx,cy);
  uint8_t d=wipeDist(cx);
  if(d!=255){
    if(!wipeRippling){
      if(d==wipeProg){
        return TILE_PACICON;
      }
      if(d<wipeProg){
        return pgm_read_byte(&cellTileFilled[kind]);
      }
    }else if(d>=wipeRipple){
      return pgm_read_byte(&cellTileFilled[kind]);
    }
  }
  return pgm_read_byte(&cellTile[kind]);
}

void loadHalf(uint8_t half, uint8_t mapIx){
  halfMap[half]=mapIx;
  dotsLeft[half]=0;
  for(uint8_t i=0;i<sizeof(dotBits[0]);i++){
    dotBits[half][i]=0;
  }
  for(uint16_t ix=0;ix<MAP_SIZE;ix++){
    uint8_t k=mapKind(mapIx,ix);
    if(k==K_DOT || k==K_PELLET){
      dotBits[half][ix>>3]|=(1<<(ix&7));
      ++dotsLeft[half];
    }
  }
}

// ---- Title screen band and high score page tiles --------------------------------------------------------------
// The characters that run across the title screen are tile images, so the band can be scrolled like the maze

typedef struct BandItem{
  uint8_t x;      // tile column within the scene
  uint8_t row;    // tile row within the band
  uint8_t kind;   // 0=Pac-Man, 1=ghost, 2=frightened ghost, 3=big Pac-Man, 4=big ghost
} BandItem;

const BandItem sceneA[] PROGMEM={ {0,2,1},{4,2,0} };                                   // a ghost chasing Pac-Man
const BandItem sceneB[] PROGMEM={ {0,2,2},{4,1,3} };                                   // big Pac-Man chasing a frightened ghost
const BandItem sceneC[] PROGMEM={ {0,1,4},{6,1,4},{11,2,0} };                           // two big ghosts chasing Pac-Man
const uint8_t sceneCount[3] PROGMEM={2,2,3};
const uint8_t sceneWidth[3] PROGMEM={6,8,13};
#define BAND_X      20      // where in the band's world the scene sits

uint8_t bandTileAt(uint8_t col, uint8_t row){
  const BandItem *items;
  switch(bandScene){
    case 0: items=sceneA; break;
    case 1: items=sceneB; break;
    default: items=sceneC; break;
  }
  uint8_t f=(tick>>2)&1;
  for(uint8_t i=0;i<pgm_read_byte(&sceneCount[bandScene]);i++){
    uint8_t x=pgm_read_byte(&items[i].x)+BAND_X;
    uint8_t r=pgm_read_byte(&items[i].row);
    uint8_t w=2,h=2,off;
    switch(pgm_read_byte(&items[i].kind)){
      case 0: off=f?CHR_PAC_R1:CHR_PAC_R0; break;
      case 1: off=f?CHR_GHOST1:CHR_GHOST0; break;
      case 2: off=f?CHR_SCARED1:CHR_SCARED0; break;
      case 3: off=f?CHR_BIGPAC_SHUT:CHR_BIGPAC_OPEN; w=4; h=4; break;
      default: off=f?CHR_BIGGHOST1:CHR_BIGGHOST0; w=4; h=4; break;
    }
    if(col>=x && col<x+w && row>=r && row<r+h){
      return pgm_read_byte(&charTiles[off+((row-r)*w)+(col-x)]);
    }
  }
  return BLANK_TILE;
}

/// @brief One tile of the high score page. The page sits in the right half of a 64 column world, so scrolling the
/// camera along slides it on and then off to the left
uint8_t rankTileAt(uint8_t col, uint8_t row){
  if(col<32){
    return BLANK_TILE;
  }
  uint8_t pc=col-32;
  if(row<3){
    return pgm_read_byte(&rankHeading[row][pc]);
  }
  if(row<6 || (row&1) || row>24){
    return BLANK_TILE;
  }
  // The list sits one column to the right of where the numbers first land, so that it lines up under the heading
  pc--;
  uint8_t i=(row-6)>>1;
  if(i==rankBlank){
    return BLANK_TILE;     // this entry is blinked off for the moment
  }
  if(pc==4 || pc==5){
    uint8_t n=i+1;
    if(pc==4){
      return (n>=10)?(n/10):BLANK_TILE;
    }
    return n%10;
  }
  if(pc==7){
    return TILE_COLON;
  }
  if(pc>=9 && pc<16){
    uint32_t p=1;
    for(uint8_t j=0;j<15-pc;j++){
      p*=10;
    }
    return (scoreAt(i)/p)%10;
  }
  if(pc>=18 && pc<21){
    uint8_t c=nameCharAt(i,pc-18);
    return (c<26)?(TILE_A+c):BLANK_TILE;
  }
  if(pc==23){
    return TILE_PACICON;
  }
  return BLANK_TILE;
}

// ---- Tilemap scrolling -----------------------------------------------------------------------------------------

uint8_t scrollTile(uint8_t col, uint8_t row){
  switch(scrollMode){
    case SM_BAND: return bandTileAt(col,row);
    case SM_RANK: return rankTileAt(col,row);
    default:      return cellTileAt(col,row);
  }
}

/// @brief Write the world column "worldCol" into screen RAM column "col"
void fillColumn(uint8_t col, uint8_t worldCol){
  uint8_t *p=screenRam+(tileRowStart*BYTES_PER_BUFFER_LINE)+col;
  for(uint8_t r=0;r<scrollRows;r++){
    *p=scrollTile(worldCol,r);
    p+=BYTES_PER_BUFFER_LINE;
  }
}

/// @brief Redraw the entire visible window from the world. Slowish (about a frame) so only used when jumping
void drawFullView(){
  viewTC=camX>>3;
  for(uint8_t c=0;c<BYTES_PER_BUFFER_LINE;c++){
    fillColumn(c,(viewTC+c)%scrollW);
  }
  setScroll(camX&0x07,0);
}

/// @brief Scroll the screen by a few pixels (positive = view moves right)
void scrollBy(int8_t d){
  camX=wrapTo(camX+d,((int16_t)scrollW)<<3);
  uint8_t ntc=camX>>3;
  while(viewTC!=ntc){
    if(d>0){
      shiftTiles(-1,BLANK_TILE);
      viewTC=(viewTC+1==scrollW)?0:viewTC+1;
      fillColumn(BYTES_PER_BUFFER_LINE-1,(viewTC+(BYTES_PER_BUFFER_LINE-1))%scrollW);
    }else{
      shiftTiles(1,BLANK_TILE);
      viewTC=(viewTC==0)?scrollW-1:viewTC-1;
      fillColumn(0,viewTC);
    }
  }
  setScroll(camX&0x07,0);
}

int16_t cameraTarget(){
  return wrapTo(pacX+SPRITE_OFFSET-128,WORLD_PX_W);
}

void snapCamera(){
  camX=cameraTarget();
  drawFullView();
}

void followPac(){
  int16_t d=wrapDelta(cameraTarget()-camX,WORLD_PX_W);
  d=constrain(d,-3,3);
  if(d){
    scrollBy(d);
  }
}

/// @brief Screen RAM column that a world column is currently shown in, or 255 if it is off screen
uint8_t screenColumnOf(uint8_t worldCol){
  uint8_t sc=(worldCol+scrollW-viewTC)%scrollW;
  return (sc<BYTES_PER_BUFFER_LINE)?sc:255;
}

void redrawWorldColumn(uint8_t worldCol){
  uint8_t sc=screenColumnOf(worldCol);
  if(sc!=255){
    fillColumn(sc,worldCol);
  }
}

void redrawCell(uint8_t cx, uint8_t cy){
  uint8_t sc=screenColumnOf(cx);
  if(sc!=255){
    screenRam[((tileRowStart+cy)*BYTES_PER_BUFFER_LINE)+sc]=cellTileAt(cx,cy);
  }
}

// ---- Ghosts ----------------------------------------------------------------------------------------------------
// All four ghosts live in the box in the centre of the world. They bounce around in there until their release timer
// runs out, leave through the gate, and then chase Pac-Man (each with its own idea of where to aim), occasionally
// retreating to their own corner. Eaten ghosts return to the box as eyes.

// ---- Score for an eaten ghost flying up to the score display ---------------------------------------------------
// The ghost shows its score while the game is paused for a moment. When play resumes the ghost carries on as eyes, and
// the number carries on up to the score on a sprite of its own.

/// @brief Add the flying score to the score now (when it arrives, or if something else interrupts it)
void finishScoreFly(){
  if(scoreFlyTimer){
    score+=((uint32_t)SCORE_GHOST)*(scoreFlyIdx+1);   // 400, 800, 1200 ... up to 3200
    hudDirty=1;
    scoreFlyTimer=0;
  }
}

void startScoreFly(Ghost *g){
  finishScoreFly();      // (only one can be in the air at a time)
  int16_t sx=wrapTo(g->x-SPRITE_OFFSET-camX,WORLD_PX_W);
  scoreFlyX=constrain(sx,0,240);
  scoreFlyY=constrain(VIEW_TOP+g->y-SPRITE_OFFSET,0,240);
  scoreFlyIdx=g->chain;
  scoreFlyTimer=SCORE_FLIGHT;
}

void scoreFlyUpdate(){
  if(scoreFlyTimer && --scoreFlyTimer==0){
    scoreFlyTimer=1;     // (finishScoreFly adds it up and clears the timer)
    finishScoreFly();
  }
}

uint8_t cellDistance(int16_t ax, int16_t ay, int16_t bx, int16_t by){
  return abs(wrapDelta(ax-bx,WORLD_W))+abs(wrapDelta(ay-by,WORLD_H));
}

/// @brief Find the open cell nearest to a (probably now solid) cell
bool nearestWalkable(int8_t cx, int8_t cy, int8_t *ox, int8_t *oy){
  for(int8_t r=0;r<=10;r++){
    for(int8_t dy=-r;dy<=r;dy++){
      int8_t rem=r-abs(dy);
      for(int8_t s=-1;s<=1;s+=2){
        int8_t dx=s*rem;
        if(walkable(cx+dx,cy+dy)){
          *ox=wrapTo(cx+dx,WORLD_W);
          *oy=wrapTo(cy+dy,WORLD_H);
          return true;
        }
        if(rem==0){
          break;
        }
      }
    }
  }
  return false;
}

/// @brief Put a ghost back in the box, to be released after "delay" frames
void ghostToPen(uint8_t gi, uint16_t delay){
  Ghost *g=&ghosts[gi];
  g->x=PEN_X0+(gi*(PEN_X1-PEN_X0)/(NUM_GHOSTS-1));
  g->y=(gi&1)?PEN_Y1:PEN_Y0;
  g->dir=(gi&1)?DIR_UP:DIR_DOWN;
  g->state=GS_PEN;
  g->timer=delay;
}

void resetGhosts(){
  static const uint16_t releaseDelay[NUM_GHOSTS] PROGMEM={100,300,550,800};
  for(uint8_t i=0;i<NUM_GHOSTS;i++){
    ghostToPen(i,pgm_read_word(&releaseDelay[i]));
  }
}

/// @brief Choose a new direction for a ghost sitting exactly on a cell
/// @return false if there is nowhere to go
bool ghostChooseDir(Ghost *g, uint8_t gi){
  static const int8_t scatterX[NUM_GHOSTS] PROGMEM={WORLD_W-1,0,WORLD_W-1,0};
  static const int8_t scatterY[NUM_GHOSTS] PROGMEM={0,0,WORLD_H-1,WORLD_H-1};
  int8_t cx=g->x>>3;
  int8_t cy=g->y>>3;
  uint8_t back=(g->dir+2)&3;
  int16_t tx,ty;
  if(g->state==GS_EYES){
    tx=EXIT_CX;
    ty=EXIT_CY;
  }else{
    int16_t px=pacX>>3;
    int16_t py=pacY>>3;
    uint16_t elapsed=(GAME_SECONDS*50)-framesLeft;
    if(g->state!=GS_SCARED && (elapsed%CHASE_CYCLE)<SCATTER_FRAMES){
      tx=(int8_t)pgm_read_byte(&scatterX[gi]);
      ty=(int8_t)pgm_read_byte(&scatterY[gi]);
    }else{
      switch(gi){
        case 1:
          // Aims ahead of Pac-Man
          tx=px+DX(pacDir)*4;
          ty=py+DY(pacDir)*4;
        break;
        case 2:
          // Aims at the far side of Pac-Man from the first ghost, which tends to trap him between them
          tx=2*(px+DX(pacDir)*2)-(ghosts[0].x>>3);
          ty=2*(py+DY(pacDir)*2)-(ghosts[0].y>>3);
        break;
        case 3:
          // Chases from a distance, but loses its nerve when close
          if(cellDistance(cx,cy,px,py)<8){
            tx=(int8_t)pgm_read_byte(&scatterX[gi]);
            ty=(int8_t)pgm_read_byte(&scatterY[gi]);
          }else{
            tx=px;
            ty=py;
          }
        break;
        default:
          tx=px;
          ty=py;
        break;
      }
    }
  }
  static const uint8_t order[4] PROGMEM={DIR_UP,DIR_LEFT,DIR_DOWN,DIR_RIGHT};
  uint8_t cand[4];
  uint8_t n=0;
  uint8_t best=DIR_NONE;
  int16_t bestDist=32767;
  for(uint8_t i=0;i<4;i++){
    uint8_t d=pgm_read_byte(&order[i]);
    if(d==back){
      continue;
    }
    int8_t nx=cx+DX(d);
    int8_t ny=cy+DY(d);
    if(!walkable(nx,ny)){
      continue;
    }
    cand[n++]=d;
    int16_t ddx=wrapDelta(nx-tx,WORLD_W);
    int16_t ddy=wrapDelta(ny-ty,WORLD_H);
    int16_t dist=ddx*ddx+ddy*ddy;
    if(dist<bestDist){
      bestDist=dist;
      best=d;
    }
  }
  if(n==0){
    // dead end
    if(!walkable(cx+DX(back),cy+DY(back))){
      return false;
    }
    best=back;
  }else if(g->state==GS_SCARED){
    best=cand[rand8()%n];
  }
  g->dir=best;
  return true;
}

void moveGhost(Ghost *g, uint8_t gi, uint8_t dist){
  while(dist){
    if(g->state==GS_EYES && g->x==((int16_t)EXIT_CX<<3) && g->y==((int16_t)EXIT_CY<<3)){
      // Back above the gate. Checked every time a cell is crossed, as a frightened ghost that has been eaten can be
      // an odd number of pixels off the grid and so never ends a frame exactly on this cell
      g->state=GS_ENTER;
      g->x=GATE_X;
      return;
    }
    if(((g->x|g->y)&7)==0){
      if(!ghostChooseDir(g,gi)){
        return;
      }
    }
    uint8_t rem;
    switch(g->dir){
      case DIR_RIGHT: rem=8-(g->x&7); break;
      case DIR_LEFT:  rem=(g->x&7)?(g->x&7):8; break;
      case DIR_DOWN:  rem=8-(g->y&7); break;
      default:        rem=(g->y&7)?(g->y&7):8; break;
    }
    uint8_t s=(dist<rem)?dist:rem;
    g->x=wrapTo(g->x+DX(g->dir)*s,WORLD_PX_W);
    g->y=wrapTo(g->y+DY(g->dir)*s,WORLD_PX_H);
    dist-=s;
  }
}

void updateGhosts(){
  for(uint8_t i=0;i<NUM_GHOSTS;i++){
    Ghost *g=&ghosts[i];
    switch(g->state){

      case GS_PEN:
        // Bounce up and down inside the box until it is time to leave
        if(g->dir==DIR_UP){
          if(--g->y<=PEN_Y0){ g->dir=DIR_DOWN; }
        }else{
          if(++g->y>=PEN_Y1){ g->dir=DIR_UP; }
        }
        if(g->timer && --g->timer==0){
          g->state=GS_LEAVING;
        }
      continue;

      case GS_LEAVING:
        // Slide across to under the gate, then straight up and out
        if(g->x<GATE_X){ ++g->x; }
        else if(g->x>GATE_X){ --g->x; }
        else if(g->y>EXIT_Y){ --g->y; }
        else{
          g->state=(scareTimer?GS_SCARED:GS_AWAKE);
          g->dir=DIR_UP;
        }
      continue;

      case GS_SCORE:
        // The pause is over: the number takes off on its own sprite, and the ghost carries on as eyes
        startScoreFly(g);
        g->state=GS_EYES;
      continue;

      case GS_ENTER:
        // Eyes go back down through the gate and the ghost starts over in the box
        g->y+=2;
        if(g->y>=((PEN_Y0+PEN_Y1)/2)){
          ghostToPen(i,90);
          g->x=GATE_X;
        }
      continue;

      default:
      break;
    }

    if(g->state==GS_NONE){
      continue;
    }
    if(((g->x|g->y)&7)==0 && !walkable(g->x>>3,g->y>>3)){
      // The maze was replaced under it - move to the nearest open alley
      int8_t ox,oy;
      if(nearestWalkable(g->x>>3,g->y>>3,&ox,&oy)){
        g->x=(int16_t)ox<<3;
        g->y=(int16_t)oy<<3;
      }
    }

    uint8_t dist;
    switch(g->state){
      case GS_AWAKE:
        dist=((tick&1)?1:2);
        if(mapSeq>=6){
          dist=2;
        }
      break;

      case GS_SCARED:
        dist=((tick&1)?0:1);
      break;

      default: // GS_EYES
        dist=2;
      break;
    }
    moveGhost(g,i,dist);
  }
}

// ---- Fruit and half-map replacement -----------------------------------------------------------------------

void spawnFruit(){
  // The fruit appears in the half that has NOT been cleared, in the lane right next to the pen and half way down it
  uint8_t half=((clearedMask&1)?1:0);
  fruitCX=(half==0)?(MAP_W-1):RIGHT_HALF_X;
  fruitCY=MAP_H/2;
  fruitType=(fruitsEaten<NUM_ICONS)?fruitsEaten:(NUM_ICONS-1);   // after the crown, it stays the crown
  fruitActive=1;
}

/// @brief If Pac-Man ends up inside a wall because the map under him was replaced, move him to the nearest open cell
void rescuePac(){
  int8_t cx=pacX>>3;
  int8_t cy=pacY>>3;
  if(walkable(cx,cy)){
    return;
  }
  int8_t ox,oy;
  if(nearestWalkable(cx,cy,&ox,&oy)){
    pacX=(int16_t)ox<<3;
    pacY=(int16_t)oy<<3;
  }
}

/// @brief Begin replacing a cleared half with the next map. A column of Pac-Man tiles sweeps away from the centre of
/// the world, replacing the half as it goes. The new maze is shown "filled in" behind the sweep, and once it has
/// finished a ripple turns it back to normal
void startNextWipe(){
  if(wipeHalf!=255 || pendingWipe==0){
    return;
  }
  uint8_t half=(pendingWipe&1)?0:1;
  pendingWipe&=~(1<<half);
  wipeHalf=half;
  wipeProg=0;
  wipeRippling=0;
  wipeRipple=0;
  // Even maps are the left halves, odd maps the mirrored right halves of the same maze
  wipeNewMap=((mapSeq&7)<<1)+half;
  ++mapSeq;
  redrawWorldColumn(wipeColumn(0));
}

void wipeStep(){
  uint8_t half=wipeHalf;
  if(wipeRippling){
    // The ripple turns the filled in tiles back to normal, moving away from the centre
    uint8_t col=wipeColumn(wipeRipple);
    ++wipeRipple;
    redrawWorldColumn(col);
    if(wipeRipple>=MAP_W){
      halfMap[half]=wipeNewMap;
      wipeHalf=255;
      wipeRippling=0;
      startNextWipe();
    }
    return;
  }

  uint8_t lx=(half==0)?(MAP_W-1-wipeProg):wipeProg;
  // Switch this column over to the new map - rebuild its dots
  for(uint8_t cy=0;cy<MAP_H;cy++){
    uint16_t ix=((uint16_t)cy*MAP_W)+lx;
    uint8_t mask=(1<<(ix&7));
    if(dotBits[half][ix>>3]&mask){
      dotBits[half][ix>>3]&=~mask;
      --dotsLeft[half];
    }
    uint8_t k=mapKind(wipeNewMap,ix);
    if(k==K_DOT || k==K_PELLET){
      dotBits[half][ix>>3]|=mask;
      ++dotsLeft[half];
    }
  }
  uint8_t doneCol=wipeColumn(wipeProg);
  ++wipeProg;
  redrawWorldColumn(doneCol);
  if(wipeProg>=MAP_W){
    wipeRippling=1;
    wipeRipple=0;
  }else{
    redrawWorldColumn(wipeColumn(wipeProg));
  }
}

void eatFruit(){
  score+=pgm_read_word(&fruitScore[fruitType]);
  ++fruitsEaten;
  sfxStart(SFX_FRUIT,20);
  fruitActive=0;
  pendingWipe|=clearedMask;
  clearedMask=0;
  startNextWipe();
  hudDirty=1;
}

// ---- Pac-Man ---------------------------------------------------------------------------------------------------

void scareGhosts(){
  if(scareTimer==0){
    ghostChain=0;     // a new pellet only restarts the 400, 800 ... 3200 chain if the last one has worn off
  }
  scareTimer=SCARE_FRAMES;
  for(uint8_t i=0;i<NUM_GHOSTS;i++){
    if(ghosts[i].state==GS_AWAKE){
      ghosts[i].state=GS_SCARED;
      ghosts[i].dir=(ghosts[i].dir+2)&3;
    }
  }
}

void pacEat(uint8_t cx, uint8_t cy){
  uint8_t half;
  uint8_t lx;
  if(cx<MAP_W){
    half=0;
    lx=cx;
  }else if(cx>=RIGHT_HALF_X){
    half=1;
    lx=cx-RIGHT_HALF_X;
  }else{
    return;   // The centre has nothing to eat or collect
  }
  uint16_t ix=((uint16_t)cy*MAP_W)+lx;
  uint8_t mask=(1<<(ix&7));
  if(dotBits[half][ix>>3]&mask){
    dotBits[half][ix>>3]&=~mask;
    if(mapKind(mapFor(half,lx),ix)==K_PELLET){
      score+=SCORE_PELLET;
      trailAdd(cx,cy,SCORE_PELLET/10);
      sfxLow=1;
      scareGhosts();
    }else{
      // Dots are worth more the more of them have been eaten without dying
      uint8_t tens=1+(dotsEaten/DOT_VALUE_STEP);
      if(tens>5){
        tens=5;
      }
      score+=tens*10;
      trailAdd(cx,cy,tens);
      sfxLow=0;
    }
    ++dotsEaten;
    hudDirty=1;
    redrawCell(cx,cy);
    sfxToggle^=1;
    sfxStart(SFX_CHOMP,4);
    if(--dotsLeft[half]==0 && wipeHalf!=half){
      clearedMask|=(1<<half);
      if(!fruitActive){
        spawnFruit();
      }
    }
  }
}

void updatePac(){
  if(demoMode){
    // The demo drives Pac-Man itself, choosing at each junction (below)
  }else if(PRESSING_UP){ pacWant=DIR_UP; }
  else if(PRESSING_RIGHT){ pacWant=DIR_RIGHT; }
  else if(PRESSING_DOWN){ pacWant=DIR_DOWN; }
  else if(PRESSING_LEFT){ pacWant=DIR_LEFT; }
  else{ pacWant=DIR_NONE; }   // Pac-Man just keeps going the way he was facing until he hits a wall

  uint8_t dist=2;
  while(dist){
    if(((pacX|pacY)&7)==0){
      rescuePac();
      uint8_t cx=pacX>>3;
      uint8_t cy=pacY>>3;
      pacEat(cx,cy);
      if(fruitActive && cx==fruitCX && cy==fruitCY){
        eatFruit();
      }
      if(demoMode){
        // Wander: pick any way out of this cell except back the way we came, unless it's a dead end
        uint8_t cand[4];
        uint8_t n=0;
        for(uint8_t d=0;d<4;d++){
          if(d!=((pacDir+2)&3) && walkable(cx+DX(d),cy+DY(d))){
            cand[n++]=d;
          }
        }
        pacWant=(n?cand[rand8()%n]:((pacDir+2)&3));
      }
      if(pacWant!=DIR_NONE && walkable(cx+DX(pacWant),cy+DY(pacWant))){
        pacDir=pacWant;
      }
      if(!walkable(cx+DX(pacDir),cy+DY(pacDir))){
        return;   // stopped against a wall
      }
    }else if(pacWant==((pacDir+2)&3)){
      pacDir=pacWant;   // reversing is allowed anywhere
    }
    uint8_t rem;
    switch(pacDir){
      case DIR_RIGHT: rem=8-(pacX&7); break;
      case DIR_LEFT:  rem=(pacX&7)?(pacX&7):8; break;
      case DIR_DOWN:  rem=8-(pacY&7); break;
      default:        rem=(pacY&7)?(pacY&7):8; break;
    }
    uint8_t s=(dist<rem)?dist:rem;
    pacX=wrapTo(pacX+DX(pacDir)*s,WORLD_PX_W);
    pacY=wrapTo(pacY+DY(pacDir)*s,WORLD_PX_H);
    dist-=s;
    ++pacAnim;
  }
}

// ---- Collisions ------------------------------------------------------------------------------------------------

void checkCollisions(){
  for(uint8_t i=0;i<NUM_GHOSTS;i++){
    Ghost *g=&ghosts[i];
    if(g->state!=GS_AWAKE && g->state!=GS_SCARED){
      continue;
    }
    if(abs(wrapDelta(g->x-pacX,WORLD_PX_W))<7 && abs(wrapDelta(g->y-pacY,WORLD_PX_H))<7){
      if(g->state==GS_SCARED){
        // The ghost turns into its score (400, 800, 1200 ... 3200) which flies up to the score and is added there
        g->chain=(ghostChain<7?ghostChain:7);
        ++ghostChain;
        sfxStart(SFX_GHOST,16);
        freezeTimer=GHOST_EAT_FREEZE;
        g->state=GS_SCORE;
        g->timer=SCORE_FLIGHT;
      }else{
        setState(dying);
        return;
      }
    }
  }
}

// ---- Sprites ---------------------------------------------------------------------------------------------------

void placeSpriteMasked(uint8_t ix, int16_t wx, int16_t wy, uint8_t def, uint8_t mask){
  int16_t sx=wrapTo(wx-SPRITE_OFFSET-camX,WORLD_PX_W);
  if(sx<0){ sx+=WORLD_PX_W; }
  int16_t sy=VIEW_TOP+wy-SPRITE_OFFSET;
  if(sy<VIEW_TOP){ sy=VIEW_TOP; }
  if(sx>240){
    resetSprite(ix);
    return;
  }
  setSpritePos(ix,sx,sy);
  setSpriteDef(ix,def,mask);
}

void placeSprite(uint8_t ix, int16_t wx, int16_t wy, uint8_t def){
  placeSpriteMasked(ix,wx,wy,def,def+SPR_MASK_OFFSET);
}

void updateSprites(){
  // While Pac-Man is dying, the ghosts and fruit disappear once the pause after the hit is over
  bool others=!(gameState==dying && dyingTimer>=DEATH_PAUSE);

  if(fruitActive && others){
    placeSpriteMasked(SPRITE_FRUIT,(int16_t)fruitCX<<3,(int16_t)fruitCY<<3,SPR_ICON_00+fruitType,SPR_ICON_MASK);
  }else{
    resetSprite(SPRITE_FRUIT);
  }

  uint8_t anim=(tick>>3)&1;
  for(uint8_t i=0;i<NUM_GHOSTS;i++){
    Ghost *g=&ghosts[i];
    uint8_t def;
    uint8_t st=g->state;
    if((st==GS_PEN || st==GS_LEAVING) && scareTimer){
      st=GS_SCARED;   // the power pellet frightens the ghosts waiting in the box too
    }
    if(!others){
      st=GS_NONE;
    }
    switch(st){
      case GS_NONE:
        resetSprite(SPRITE_GHOST0+i);
        continue;
      case GS_SCORE:
        def=SPR_SCORE_400+g->chain;     // (held still during the pause)
        break;
      case GS_ENTER:
        def=SPR_GHOST_EYES;
        break;
      case GS_PEN:
      case GS_LEAVING:
      case GS_AWAKE:
        def=SPR_GHOST_AWAKE_0+anim;
        break;
      case GS_SCARED:
        // Blink between scared and normal when the effect is about to wear off
        def=((scareTimer<120 && (tick&8))?SPR_GHOST_AWAKE_0:SPR_GHOST_SCARED_0)+anim;
        break;
      default:
        def=SPR_GHOST_EYES;
        break;
    }
    placeSprite(SPRITE_GHOST0+i,g->x,g->y,def);
  }

  // The score of an eaten ghost flying up to the score display
  if(scoreFlyTimer && others){
    int16_t k=SCORE_FLIGHT-scoreFlyTimer;
    int16_t sx=scoreFlyX+((((int16_t)SCORE_TARGET_X-scoreFlyX)*k)/SCORE_FLIGHT);
    int16_t sy=scoreFlyY+((((int16_t)SCORE_TARGET_Y-scoreFlyY)*k)/SCORE_FLIGHT);
    setSpritePos(SPRITE_SCORE,sx,sy);
    setSpriteDef(SPRITE_SCORE,SPR_SCORE_400+scoreFlyIdx,SPR_SCORE_400+scoreFlyIdx+SPR_MASK_OFFSET);
  }else{
    resetSprite(SPRITE_SCORE);
  }

  uint8_t pacDef=pgm_read_byte(&pacBaseFrame[pacDir])+((pacAnim>>2)&1);
  if(gameState==dying && dyingTimer>=DEATH_PAUSE){
    // Pac-Man folds up like the arcade game: a ball, then a mouth opening upwards until there is nothing left
    uint8_t k=(dyingTimer-DEATH_PAUSE)/DEATH_FRAME_LEN;
    if(k>=10){
      resetSprite(SPRITE_PAC);
      return;
    }
    pacDef=SPR_PAC_DIE_0+k;
  }
  placeSprite(SPRITE_PAC,pacX,pacY,pacDef);
}

// ---- Screen layouts --------------------------------------------------------------------------------------------

/// @brief Layout for playing: three static rows of HUD at the top, then the scrolling maze, then a few more static rows
void setLayoutGame(){
  resetSprites();
  setTileRowSplit(VIEW_TOP,VIEW_BOTTOM);
  clearTileMap(BLANK_TILE);
  selectTileBank(BANK_GAME);
  camX=0;
  viewTC=0;
  scrollMode=SM_GAME;
  scrollW=WORLD_W;
  scrollRows=WORLD_H;
  setScroll(0,0);
}

/// @brief A layout where the whole screen is one scrolling area (so nothing is static) using the given tile bank
void setLayoutFull(uint8_t bank){
  resetSprites();
  setTileRowSplit(1,248);   // (not 0: the engine only switches to the scroll position once its line counter reaches topY+1)
  // (Horizontal scroll position 7 is the one where the scrolling area lines up with static areas; 0 would shift it right by 7)
  clearTileMap(BLANK_TILE);
  selectTileBank(bank);
  camX=0;
  viewTC=0;
  scrollMode=SM_GAME;
  scrollW=WORLD_W;
  scrollRows=WORLD_H;
  setScroll(7,0);
}

// ---- Title screen ----------------------------------------------------------------------------------------------
// 1) The logo scrolls up the screen. 2) The stripes open out from the middle. 3) Characters run along the band between
// the stripes, with "PRESS START BUTTON" flashing underneath, and then it moves on to the demo.

#define TITLE_SCROLL_END    248
#define SCENE_COUNT         3

void titleFillRow(uint8_t worldRow){
  // The logo sits TITLE_LOGO_ROW rows down the page, which starts 31 rows below the screen
  uint8_t *p=screenRam+((worldRow%32)*BYTES_PER_BUFFER_LINE);
  int8_t j=worldRow-(TITLE_LOGO_ROW+31);
  for(uint8_t c=0;c<BYTES_PER_BUFFER_LINE;c++){
    *p++=(j>=0 && j<TITLE_LOGO_ROWS)?pgm_read_byte(&titleLogo[j][c]):BLANK_TILE;
  }
}

void titleStart(){
  setLayoutFull(BANK_TITLE);
  attractPhase=0;
  attractTimer=0;
  audioMute();
}

/// @brief Switch to the layout with the scrolling band between static top and bottom parts, and draw the static parts
void titleEnterBand(){
  setTileRowSplit(BAND_TOP,BAND_BOTTOM);
  clearTileMap(BLANK_TILE);
  selectTileBank(BANK_TITLE);
  for(uint8_t j=0;j<TITLE_LOGO_ROWS;j++){
    for(uint8_t c=0;c<BYTES_PER_BUFFER_LINE;c++){
      screenRam[((TITLE_LOGO_ROW+j)*BYTES_PER_BUFFER_LINE)+c]=pgm_read_byte(&titleLogo[j][c]);
    }
  }
  scrollMode=SM_BAND;
  scrollW=BAND_W;
  scrollRows=BAND_ROWS;
  bandScene=0;
  camX=0;
  viewTC=0;
  setScroll(0,0);
}

void titleStartScene(){
  attractTimer=0;
  // The camera starts far enough along that the scene is just off screen on the side it enters from
  switch(bandScene){
    case 0: camX=(BAND_X+pgm_read_byte(&sceneWidth[0]))*8; break;                 // moving right
    case 1: camX=wrapTo((BAND_X-32)*8,BAND_W*8); break;           // moving left
    default: camX=(BAND_X+pgm_read_byte(&sceneWidth[2]))*8; break;                // moving right again
  }
  drawFullView();
}

void titleLoop(){
  if(PRESSED_START){
    setState(ready);
    return;
  }

  switch(attractPhase){

    case 0:
    {
      // Scroll the logo up. Rows enter at the bottom of the screen as they come into view
      uint8_t oldRow=attractTimer>>3;
      attractTimer+=2;
      uint8_t newRow=attractTimer>>3;
      for(uint8_t r=oldRow+1;r<=newRow;r++){
        titleFillRow(r+31);
      }
      setScroll(7,attractTimer);
      if(attractTimer>=TITLE_SCROLL_END){
        titleEnterBand();
        attractPhase=1;
        attractTimer=0;
      }
    }
    break;

    case 1:
    {
      // The stripes open out from the middle of the screen
      for(int8_t c=15-attractTimer;c<=16+(int8_t)attractTimer;c++){
        if(c>=0 && c<32){
          screenRam[(13*BYTES_PER_BUFFER_LINE)+c]=TILE_STRIPE_TOP;
          screenRam[(14*BYTES_PER_BUFFER_LINE)+c]=TILE_STRIPE_BOT;
        }
      }
      if(++attractTimer>=17){
        for(uint8_t j=0;j<TITLE_BOTTOM_ROWS;j++){
          for(uint8_t c=0;c<BYTES_PER_BUFFER_LINE;c++){
            screenRam[((18+j)*BYTES_PER_BUFFER_LINE)+c]=pgm_read_byte(&titleBottom[j][c]);
          }
        }
        attractPhase=2;
        titleStartScene();
      }
    }
    break;

    default:
    {
      // Flash PRESS START BUTTON
      if((tick&31)==0){
        if(tick&32){
          clearBlock(7,16,18);
        }else{
          drawStr_P(PSTR("PRESS START BUTTON"),7,16);
        }
      }
      // Run the current scene across the band
      static const uint8_t sceneLen[SCENE_COUNT] PROGMEM={152,160,180};
      scrollBy(bandScene==1?2:-2);
      if((tick&3)==0){
        drawFullView();      // swap the animation frames
      }
      if(++attractTimer>=pgm_read_byte(&sceneLen[bandScene])){
        if(++bandScene>=SCENE_COUNT){
          setState(demo);
        }else{
          titleStartScene();
        }
      }
    }
    break;
  }
}

// ---- High scores -----------------------------------------------------------------------------------------------

void rankingStart(){
  setLayoutFull(BANK_SCORES);
  rankBlank=255;
  rankFrame=0;
  scrollMode=SM_RANK;
  scrollW=RANK_W;
  scrollRows=31;
  camX=1;      // odd positions only, so it can stop on 263 (tile column 32, scroll position 7) where the page lines up exactly
  attractTimer=0;
  attractPhase=0;
  drawFullView();
  audioMute();
}

/// @brief Redraw the visible part of a high score entry
void rankRedrawEntry(uint8_t i){
  uint8_t row=6+(i*2);
  for(uint8_t c=0;c<BYTES_PER_BUFFER_LINE;c++){
    screenRam[((tileRowStart+row)*BYTES_PER_BUFFER_LINE)+c]=rankTileAt((viewTC+c)%scrollW,row);
  }
}

void rankingLoop(){
  if(PRESSED_START){
    setState(ready);
    return;
  }
  // Each entry in turn blinks off for half a second, working down the table and round again
  if(++rankFrame>=RANK_BLINK_FRAMES){
    rankFrame=0;
    uint8_t old=rankBlank;
    rankBlank=(rankBlank>=NUM_SCORES-1)?0:rankBlank+1;     // (255 + 1 wraps to 0 too, so the first blink is the top entry)
    if(old<NUM_SCORES){
      rankRedrawEntry(old);
    }
    rankRedrawEntry(rankBlank);
  }
  // The page slides in from the right, stays put for a while, then carries on off to the left
  if(camX==263 && attractPhase<RANK_HOLD_FRAMES){
    ++attractPhase;
    return;
  }
  scrollBy(2);
  attractTimer+=2;
  if(attractTimer>=RANK_W*8){
    setState(title);
  }
}

// ---- Name entry ------------------------------------------------------------------------------------------------
// 1) The CONGRATULATIONS heading scrolls up the screen. 2) PLEASE ENTER YOUR NAME is typed out a character a frame.
// 3) Everything else appears, and the name can be entered: up/down changes a letter, left/right moves along, and
// moving onto the ghost enters the name.

#define ENTRY_Y             172
#define ENTRY_X(pos)        (84+((pos)*32))     // sprites centred under the letter in tile column 12+4*pos
#define ENTRY_MESSAGE_LEN   22

void nameEntryDrawLetters(){
  for(uint8_t i=0;i<3;i++){
    uint8_t c=entryName[i];
    drawChar((c<26)?(TILE_A+c):BLANK_TILE,12+(i*4),19);
    drawChar(TILE_UNDERSCORE,12+(i*4),20);
  }
}

void nameEntryFillRow(uint8_t worldRow){
  // The heading sits 3 rows down the page, which starts 31 rows below the screen
  uint8_t *p=screenRam+((worldRow%32)*BYTES_PER_BUFFER_LINE);
  int8_t j=worldRow-(3+31);
  for(uint8_t c=0;c<BYTES_PER_BUFFER_LINE;c++){
    *p++=(j>=0 && j<4)?pgm_read_byte(&congratsHeading[j][c]):BLANK_TILE;
  }
}

/// @brief Everything that appears once the message has been typed out
void nameEntryDrawRest(){
  for(uint8_t c=0;c<BYTES_PER_BUFFER_LINE;c++){
    drawChar(TILE_SCORE_LINE,c,13);
    drawChar(TILE_SCORE_LINE,c,25);
  }
  uint8_t n=entryRank+1;
  drawNumPad(n,9,16,2);
  drawStr_P(PSTR("TH :"),11,16);
  drawNumPad(score,16,16,7);
  nameEntryDrawLetters();
}

void nameEntryStart(){
  setLayoutFull(BANK_SCORES);
  audioMute();
  attractPhase=0;
  attractTimer=0;
  entryName[0]=entryName[1]=entryName[2]=0;
  entryPos=0;
}

void nameEntryLoop(){
  if(attractPhase==0){
    // Scroll the heading up. Rows enter at the bottom of the screen as they come into view
    uint8_t oldRow=attractTimer>>3;
    attractTimer+=2;
    uint8_t newRow=attractTimer>>3;
    for(uint8_t r=oldRow+1;r<=newRow;r++){
      nameEntryFillRow(r+31);
    }
    setScroll(7,attractTimer);
    if(attractTimer>=TITLE_SCROLL_END){
      // Landed. Redraw it in the ordinary layout (it looks identical) so the rest can be drawn at plain positions
      clearTileMap(BLANK_TILE);
      for(uint8_t r=0;r<4;r++){
        for(uint8_t c=0;c<BYTES_PER_BUFFER_LINE;c++){
          screenRam[((3+r)*BYTES_PER_BUFFER_LINE)+c]=pgm_read_byte(&congratsHeading[r][c]);
        }
      }
      setScroll(7,0);
      attractPhase=1;
      attractTimer=0;
    }
    return;
  }

  if(attractPhase==1){
    // Type out the message
    static const char message[] PROGMEM="PLEASE ENTER YOUR NAME";
    drawChar(charTile(pgm_read_byte(&message[attractTimer])),5+attractTimer,9);
    if(++attractTimer>=ENTRY_MESSAGE_LEN){
      nameEntryDrawRest();
      attractPhase=2;
    }
    return;
  }

  uint8_t pos=entryPos;
  if(PRESSED_LEFT && pos>0){
    --pos;
  }else if(PRESSED_RIGHT){
    ++pos;
  }else if(pos<3){
    uint8_t c=entryName[pos];
    if(PRESSED_UP){
      entryName[pos]=(c==26)?0:c+1;
    }else if(PRESSED_DOWN){
      entryName[pos]=(c==0)?26:c-1;
    }
  }
  entryPos=pos;
  nameEntryDrawLetters();

  if(pos>=3 || PRESSED_START){
    // Reached the ghost - the name is entered
    insertScore(entryRank,score,entryName);
    setState(ranking);
    return;
  }

  // Pac-Man shows which letter is being changed, and the ghost waits at the end
  uint8_t anim=(tick>>2)&1;
  setSpritePos(SPRITE_PAC,ENTRY_X(pos),ENTRY_Y);
  setSpriteDef(SPRITE_PAC,SPR_PAC_RIGHT_0+anim,SPR_PAC_RIGHT_0+anim+SPR_MASK_OFFSET);
  setSpritePos(SPRITE_GHOST0,ENTRY_X(3),ENTRY_Y);
  setSpriteDef(SPRITE_GHOST0,SPR_GHOST_AWAKE_0+((tick>>3)&1),SPR_GHOST_AWAKE_0+((tick>>3)&1)+SPR_MASK_OFFSET);
  // Make the selected letter blink
  if(tick&8){
    drawChar(BLANK_TILE,12+(pos*4),19);
  }
}

// ---- Game flow -------------------------------------------------------------------------------------------------

void respawn(){
  trailClear();
  ghostChain=0;
  pacX=(int16_t)START_CX<<3;
  pacY=(int16_t)START_CY<<3;
  pacDir=DIR_LEFT;
  pacWant=DIR_NONE;
  pacAnim=0;
  scareTimer=0;
  resetGhosts();
  snapCamera();
}

void startGame(){
  setLayoutGame();
  score=0;
  lives=START_LIVES;
  framesLeft=GAME_SECONDS*50;
  mapSeq=1;
  fruitsEaten=0;
  clearedMask=0;
  pendingWipe=0;
  wipeHalf=255;
  wipeRippling=0;
  fruitActive=0;
  scareTimer=0;
  ghostChain=0;
  freezeTimer=0;
  scoreFlyTimer=0;
  dotsEaten=0;
  demoMode=0;
  pacX=(int16_t)START_CX<<3;
  pacY=(int16_t)START_CY<<3;
  loadHalf(0,0);
  loadHalf(1,1);
  respawn();
  drawHud();
}

// Row in screen RAM of the n'th row of the scrolling area (which is what appears below the HUD)
#define SCROLL_ROW(n)   (tileRowStart+(n))

void setState(GameState nextState){

  switch(nextState){

    case gameInit:
    {
      audioInit();
      // Holding SELECT and START as the game starts puts the original high scores back
      readInput();
      if(PRESSING_START && PRESSING_OPTION){
        scoresReset();
      }
      scoresInit();
      setLayoutFull(BANK_TITLE);
    }
    break;

    case title:
      titleStart();
    break;

    case demo:
      startGame();
      demoMode=1;
      lives=0;
      audioMute();
      drawHud();
      attractTimer=0;
    break;

    case ranking:
      rankingStart();
    break;

    case nameEntry:
      nameEntryStart();
    break;

    case ready:
      startGame();
      audioMute();
      drawStr_P(PSTR("READY"),13,4);
      introIx=0;
      introTimer=0;
    break;

    case playing:
    break;

    case dying:
      dyingTimer=0;
      finishScoreFly();     // (a score still in the air is added at once)
      dotsEaten=0;     // dots go back to being worth 10 points
      audioMute();
    break;

    case gameOver:
    {
      bool timeUp=(framesLeft==0);
      uint32_t finalScore=score;
      setLayoutFull(BANK_GAME);
      audioMute();
      drawStr_P((timeUp?PSTR("TIME UP"):PSTR("GAME OVER")),12,SCROLL_ROW(7));
      drawStr_P(PSTR("SCORE"),10,SCROLL_ROW(10));
      drawNumPad(finalScore,16,SCROLL_ROW(10),6);
    }
    break;

    default:
    break;

  }

  tick=0;
  gameState=nextState;
}

/// @brief One frame of the game itself. Used for real games and for the demo
void playFrame(){
  if(framesLeft==0){
    setState(gameOver);
    return;
  }
  if(freezeTimer){
    // Everything holds still for a moment after a ghost is eaten
    --freezeTimer;
    if(!demoMode){
      audioUpdate();
    }
    return;
  }
  --framesLeft;
  if((framesLeft%50)==0){
    hudDirty=1;
  }
  if(wipeHalf!=255 && (wipeRippling || (tick%WIPE_FRAMES)==0)){
    wipeStep();
  }
  trailUpdate();
  updatePac();
  updateGhosts();
  checkCollisions();
  if(gameState!=playing && gameState!=demo){
    return;
  }
  if(scareTimer && --scareTimer==0){
    for(uint8_t i=0;i<NUM_GHOSTS;i++){
      if(ghosts[i].state==GS_SCARED){
        ghosts[i].state=GS_AWAKE;
      }
    }
  }
  scoreFlyUpdate();
  followPac();
  updateSprites();
  if(!demoMode){
    audioUpdate();
  }
  if(hudDirty){
    drawHud();
  }
}

/// @brief Main gameloop runs at 50hz and is synchronised with display
void gameLoop(){
  // Wait until the current display frame has finished rendering
  waitFrame();

  // Update the state of the controller/button inputs
  readInput();

  ++tick;

  switch(gameState){

    case gameInit:
      setState(title);
    break;

    case title:
      rand8();
      rng^=waitFrames;
      if(rng==0){
        rng=0xACE1;
      }
      titleLoop();
    break;

    case demo:
      if(PRESSED_START){
        setState(ready);
      }else{
        playFrame();
        if(gameState==demo && ++attractTimer>=DEMO_FRAMES){
          setState(ranking);
        }
      }
    break;

    case ranking:
      rankingLoop();
    break;

    case nameEntry:
      nameEntryLoop();
    break;

    case ready:
    {
      // Hold still while the start music plays
      updateSprites();
      if(introUpdate()){
        clearBlock(13,4,5);
        audioMute();
        gameState=playing;
        tick=0;
      }
    }
    break;

    case playing:
      playFrame();
    break;

    case dying:
    {
      if(demoMode){
        // Caught in the demo - on to the high scores
        setState(ranking);
        break;
      }
      ++dyingTimer;
      if(dyingTimer==DEATH_PAUSE){
        sfxStart(SFX_DEATH,DEATH_ANIM_FRAMES);
      }
      updateSprites();
      if(dyingTimer>=DEATH_PAUSE){
        audioUpdate();
      }
      if(dyingTimer>=DEATH_PAUSE+DEATH_ANIM_FRAMES+DEATH_END_PAUSE){
        if(--lives==0){
          setState(gameOver);
        }else{
          respawn();
          hudDirty=1;
          drawHud();
          gameState=playing;
        }
      }
    }
    break;

    case gameOver:
      if(tick>GAMEOVER_FRAMES){
        entryRank=scoreRank(score);
        if(entryRank<NUM_SCORES){
          setState(nameEntry);
        }else{
          setState(title);
        }
      }
    break;

    default:
    break;
  }

  // handles sprite processing and other setup to render the next frame. You should not perform any other drawing operations after this, until the frame has rendered and the gameLoop starts again
  processFrame();

}

#endif
