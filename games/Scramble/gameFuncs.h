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

#include <EEPROM.h>
#include "gameAssets.h"
#include "gameDefs.h"
#include "engine.h"
#include "gameVars.h"

// Declarations

void setState(GameState nextState);
void gameLoop();
void drawChar(const uint8_t chr, uint8_t x, uint8_t y);
void drawText(const uint8_t *txt, uint8_t x, uint8_t y, uint8_t len);
uint8_t drawFixedText(const uint8_t *charA, uint8_t limitLen);
void drawNumber(uint16_t num, uint8_t x, uint8_t y);
void showColNumbers(void);
void drawScoreLine(void);
void addScore(uint8_t val);
void drawCredits(void);
void clearCol(uint8_t colIX, uint8_t startY,uint8_t len);
void clearBlock(uint8_t startX, uint8_t startY, uint8_t len);
void resetDisplay(void);
void doPlayPlayerAnimation(void);
void doStartAnimation(void);
void doTitleAnimation(void);
void doCompletedAnimation(void);
void updateLevelLivesUI(void);
void updateFuelUI(void);
void setLevelMetadata(void);
void resetLevel(void);
uint8_t addGroundObject(uint8_t obIX, uint8_t *tilePos);
void addLandscapeCol(void);
void updateBullets(void);
uint8_t checkPlayerVSFlying(void);
uint8_t explodeFromFlying(uint8_t charX,uint8_t startY, uint8_t checkByte);
uint8_t explodeFromTile(uint8_t *p);
void clearStructure(uint8_t *p);
uint8_t updateGroundAnimations(void);
void moveBombs(void);
uint8_t getFreeEnemySprite(void);
void launchRocket(void);
void launchSaucer(void);
void launchFireball(void);
uint8_t checkPlayerVsLandscape(void);
void runCollisionChecks(void);
void updateAudio(void);
void startAudio(uint8_t channel,const uint8_t *ptr);
void stopAudioChannel(uint8_t ix);
void stopAllAudio(void);
void doPlaying();
void drawStarField(void);

// Implementations

void drawChar(const uint8_t chr, uint8_t x, uint8_t y){
  screenRam[(y*BYTES_PER_BUFFER_LINE)+x]=chr;
}

void drawText(const uint8_t *charA, uint8_t x, uint8_t y, uint8_t len){
  for(uint8_t n=0;n<len;n++){
    drawChar(pgm_read_byte(charA++),x++,y);
  }
}

uint8_t drawFixedText(const uint8_t *charA, uint8_t limitLen){
  uint8_t x=pgm_read_byte(charA++);
  uint8_t y=pgm_read_byte(charA++);
  uint8_t len=pgm_read_byte(charA++);
  uint8_t atEnd=1;
  if(len>limitLen){
    len=limitLen;
    atEnd=0;
  }
  drawText(charA,x,y,len);
  return atEnd;
}

void drawNumber(uint16_t num, uint8_t x, uint8_t y){
  x+=4;
  while(1){
    drawChar(num%10,x--,y);
    num/=10;
    if(num==0){
      break;
    }
  }
}

void showColNumbers(void){
  for(uint8_t n=0;n<32;n++){
    drawNumber(n,0,n);
  }
}

void drawScoreLine(void){
  clearBlock(4,1,5);
  drawNumber(gV.score,4,1);
  drawNumber(hiScoreList[0],14,1);
}

void addScore(uint8_t val){
  if(IS_DEMO_MODE){
    return;
  }
  if(gV.score<1000 && (gV.score+val>=1000)){
    ++gV.lives;
    updateLevelLivesUI();
  }
  gV.score+=val;
  drawScoreLine();
}

void drawCredits(void){
  drawChar(BLANK_TILE,12,6);
  drawNumber(credits,9,6);
}

void clearCol(uint8_t colIX, uint8_t startY,uint8_t len){
  uint8_t *p=screenRam+(startY*BYTES_PER_BUFFER_LINE)+colIX;
  while(len--){
    *p=BLANK_TILE;
    p+=BYTES_PER_BUFFER_LINE;
  }
}

void clearBlock(uint8_t startX, uint8_t startY, uint8_t len){
  uint8_t *p=screenRam+(startY*BYTES_PER_BUFFER_LINE)+startX;
  while(len--){
    *p++=BLANK_TILE;
  }
}

void selectTileBank(uint8_t ix){
  currentTileData=tileData+(8*256*ix);
}

void resetDisplay(void){
  resetSprites();
  setTileRowSplit(40,232);  // 5 tiles from top, and 2 from bottom
  setScroll(0,0);
  clearTileMap(BLANK_TILE);
  drawFixedText(creditLineTxt, 255);
  drawFixedText(scoreLineTxt, 255);
  drawChar(0,9,1);
  drawChar(0,19,1);
  drawScoreLine();
  drawCredits();
}

void doPlayPlayerAnimation(void){
  if(giu==0){
    clearCol(31-gix,7,25);
    if(++gix==32){
      giu=1;
      gix=0;
      drawFixedText(playerOneTxt,255);
    }
  }else if(giu==1){
    if(++gix==100){
      setState(playing);
    }
  }
}

void doStartAnimation(void){
  if(gix==0){
    drawFixedText(pushStartButtonTxt,255);
    drawFixedText(onePlayerOnlyTxt,255);
    drawFixedText(bonusJetTxt,255);
  }
  if(PRESSED_START){
    if(credits>0){
      --credits;
      drawCredits();
      setState(playPlayer);
    }
  }
}

void doTitleAnimation(void){
  if(giu==0){
    if((gix%8)==0){
      if(giy<4){
        uint8_t yPos=7+((24+((gix+8)>>3))%25);
        uint8_t *p=screenRam+(yPos*BYTES_PER_BUFFER_LINE)+8;
        uint8_t c=48+(giy<<4);
        for(uint8_t n=0;n<16;n++){
          *p++=c++;
        }
        ++giy;
      }
    }
    if(gix>186){
      ++gix;
    }else{
      gix+=2;
    }
    setScroll(0,gix+8);
    if(gix==190 && giy==4){
      giu=1;
      gix=0;
      giy=0;
      drawFixedText(copyrightKonamiTxt,255);
    }
  }else if(giu==1){
    if(drawFixedText(howFarCanYouInvadeTxt,++gix)){
      giu=2;
      gix=0;
    }
  }else if(giu==2){
    if(drawFixedText(ourScrambleSystemTxt,++gix)){
      giu=3;
      gix=0;
    }
  }else if(giu==3){
    if(++gix==100){
      giu=4;
      gix=0;
    }
  }else if(giu==4){
    clearCol(31-gix,7,21);
    if(++gix==32){
      giu=5;
      gix=0;
      drawFixedText(scoreRankingTxt,255);
    }
  }else if(giu==5){
    if(++gix==8){
      gix=0;
      drawNumber(hiScoreList[giy],14,(giy<<1)+8);
      drawText(ptsTxt+3,19,(giy<<1)+8,5);
      drawNumber(giy+1,(giy==9?5:4),(giy<<1)+8);
      switch(giy){
        case 0:
          giv=35;
          giw=36;
          break;
        case 1:
          giv=30;
          giw=20;
          break;
        case 2:
          giv=34;
          giw=20;
          break;
        default:
          giv=36;
          giw=24;
          break;
      }
      drawChar(giv,(giy==9?10:9),(giy<<1)+8);
      drawChar(giw,(giy==9?11:10),(giy<<1)+8);
      if(++giy==10){
        giu=6;
        giy=0;
        gix=0;
      }
    }
  }else if(giu==6){
    if(++gix==150){
      giu=7;
      gix=0;
    }
  }else if(giu==7){
    clearCol(31-gix,7,21);
    clearCol(31-gix,4,1);
    if(++gix==32){
      giu=8;
      gix=0;
      giy=0;
      giw=0;
      giv=0;
      drawFixedText(scoreTableTxt,255);
    }
  }else if(giu==8){
    if(++giy==24){
      giy=0;
    }
    for(uint8_t n=0;n<6;n++){
      setSpriteDef(n,pgm_read_byte(scoreTableSpriteDef+(n*6)+(giy>>2)));
      setSpritePos(n,80,60+(n*24));
    }
    if(++gix==6){
      gix=0;
      ++giw;
      if(giw<12){
        drawFixedText(pts50Txt,giw);
      }else if(giw<24){
        drawFixedText(pts80Txt,giw-12);
      }else if(giw<36){
        drawFixedText(pts100Txt,giw-24);
      }else if(giw<48){
        drawFixedText(pts150Txt,giw-36);
      }else if(giw<60){
        drawFixedText(pts800Txt,giw-48);
      }else if(giw<72){
        drawFixedText(mysteryTxt,giw-60);
      }
    }
    if(giw>90){
      clearCol(31-giv,7,23);
      if(++giv>20){
        resetSprites();
      }
      if(giv==32){
        setState(demo);
      }
    }
  }
}

void doCompletedAnimation(void){
  if(giu==0){
    clearCol(31-gix,7,25);
    clearCol(31-gix,3,3);
    if(++gix==32){
      gix=0;
      giu=1;
      drawFixedText(congratulationsTxt,255);
    }
  }else if(giu==1){
    if(drawFixedText(youCompletedYourDutiesTxt,gix++)){
      gix=0;
      giu=2;
    }
  }else if(giu==2){
    if(drawFixedText(goodLuckNextTimeAgainTxt,gix++)){
      gix=0;
      giu=3;
    }
  }else if(giu==3){
    if(++gix==100){
      ++gV.loop;
      gV.level=0;
      if(gV.fuelDepletionRate>FASTEST_FUEL_DEPLETION_RATE){
        gV.fuelDepletionRate-=4;
      }
      setState(playPlayer);
    }
  }
}

void updateLevelLivesUI(void){
  drawFixedText(levelDisplayTxt,255);
  drawFixedText(levelCellsTxt,255);
  uint8_t *p=screenRam+(4*BYTES_PER_BUFFER_LINE)+5;
  for(uint8_t n=0;n<6;n++){
    if(gV.level>=n){
      *p++=72;
      *p++=73;
      *p++=73;
      *p++=74;
    }else{
      *p++=75;
      *p++=76;
      *p++=76;
      *p++=77;
    }
  }

  if(NOT_DEMO_MODE){

    p=screenRam+(6*BYTES_PER_BUFFER_LINE)+2;
    for(uint8_t n=0;n<6;n++){
      if(gV.lives>n){
        *p++=10;
        *p++=11;
      }else{
        *p++=BLANK_TILE;
        *p++=BLANK_TILE;
      }
    }

    switch(gV.levelType){
      case 0:
        startAudio(0,defaultAmbientSound);
      break;
      case 2:
        startAudio(0,saucerAmbientSound);
      break;
      default:
        startAudio(0,defaultAmbientSound);
      break;
    }
          
  }

  p=screenRam+(6*BYTES_PER_BUFFER_LINE)+29;
  for(uint8_t n=0;n<16;n++){
    if(gV.loop>=n){
      *p--=14;
    }else{
      *p--=BLANK_TILE;
    }
  }
}

void updateFuelUI(void){
  drawFixedText(fuelTxt,255);
  uint8_t *p=screenRam+(5*BYTES_PER_BUFFER_LINE)+9;
  *p++=106;
  uint8_t tile=71;
  uint8_t f=gV.fuel;
  for(uint8_t n=0;n<16;n++){
    if(f<8){
      tile=64+f;
    }else if(f>128){
      f=0;
      tile=76;
    }
    *p++=tile;
    f-=8;
  }
  *p=107;
}

void setPlayerPos(){
  setSpritePos(7,gV.playerX,gV.playerY);
  setSpritePos(8,gV.playerX+16,gV.playerY);
  uint8_t off=((currentFrame>>2)&0x03)<<1;
  setSpriteDef(7,off);
  setSpriteDef(8,off+1);
}

void setLevelMetadata(void){
  const uint8_t *p=levelMetaData+(5*gV.level);
  gV.levelAddr=pgm_read_byte(p)+(((uint16_t)pgm_read_byte(p+1))<<8);
  gV.levelType=pgm_read_byte(p+2);
  gV.fillBlock[0]=pgm_read_byte(p+3);
  gV.fillBlock[1]=pgm_read_byte(p+4);
}

void resetLevel(void){
  setScroll(0,0);
  setLevelMetadata();
  resetSprites();
  gV.ff=0;
  gV.fuel=128;
  gV.playerX=24;
  gV.playerY=64;
  gV.fireLockout=0;
  gV.bombLockout=0;
  gV.autoScoreInc=0;
  gV.playerAnimCounter=0;
  gV.lastFireballY=128;
  CLEAR_IS_PAUSED;
  CLEAR_LOST_LIFE;
  CLEAR_GAME_OVER;
  CLEAR_PLAYER_DEAD;
  CLEAR_HIT_BASE;
  CLEAR_OUT_OF_FUEL;
  gV.bombTmr[0]=gV.bombTmr[1]=0;

  uint8_t *d=screenRam+(27*BYTES_PER_BUFFER_LINE);
  for(uint8_t n=0;n<32;n++){
    *d=54;
    *(d+BYTES_PER_BUFFER_LINE)=60;
    *(d+BYTES_PER_BUFFER_LINE*1)=60;
    *(d+BYTES_PER_BUFFER_LINE*2)=60;
    *(d+BYTES_PER_BUFFER_LINE*3)=60;
    ++d;
    gV.colAttr[n]=0;
  }

  for(uint8_t n=0;n<MAX_BULLETS;n++){
    gV.bulletOb[n].pos=NULL;
    gV.bulletOb[n].tileIX=0;
  }

  for(uint8_t n=0;n<MAX_FLYING_ENEMIES;n++){
    gV.flyingEnemy[n].movementIX=0;
  }
}

uint8_t addGroundObject(uint8_t obIX, uint8_t *tilePos){
  if(obIX==0){
    return;
  }
  switch(obIX){
      case 1: obIX=112;break;  // Rocket
      case 2: obIX=114;break;  // Fuel
      case 4: obIX=116;break;  // Mystery
      case 8: obIX=118;break;  // Base
  }
  *tilePos=obIX+gV.ff;
  *(tilePos+BYTES_PER_BUFFER_LINE)=obIX+16+gV.ff;
  return (gV.ff==0?obIX:0);
}

/**
     * In metaData, the level type specifies the flying object types is as follows:
     * 0             Rockets (see $1CA7)
     * 2             UFOs (see $1D84)                   
     * 1             Fireballs (see $1EC6)             
     * 8             Rockets (see $1CAD)               
     * 4             None                 
     * 16            BASE
     * 
     * The level record structure is as follows: 
     *   Byte 0: used to compute LANDSCAPE_GROUND_FIRST_CHAR_PTR (see $15FD) (position in Y axis)
     *   Byte 1: sets LANDSCAPE_GROUND_FIRST_CHAR (see $160A) (actual tile index)
     *   Byte 2: used to compute LANDSCAPE_GROUND_SECOND_CHAR_PTR (see $15E4)
     *   Byte 3: sets LANDSCAPE_GROUND_SECOND_CHAR (see $15F3)
     *
     *   This is where things become a little more complex. Byte 4 determines if this record is going to be 6 or 9 bytes in size.
     *   If Byte 4 is zero:
     *       * this record is 6 bytes in size and no ceiling needs to be rendered: 
     *       * Byte 5 sets NEXT_GROUND_OBJECT_ID (see $164A) 
     *   
     *   Else If Byte 4 is nonzero:
     *       This record is 9 bytes in size because a ceiling needs to be rendered, and:
     *       * Byte 4 is used to compute LANDSCAPE_CEILING_FIRST_CHAR_PTR (see $167B)
     *       * Byte 5 is stored in LANDSCAPE_CEILING_FIRST_CHAR (see $1688)
     *       * Byte 6 is used to compute LANDSCAPE_CEILING_SECOND_CHAR_PTR (see $1664)
     *       * Byte 7 sets LANDSCAPE_CEILING_SECOND_CHAR (see $1671)
     *       * Byte 8 sets NEXT_GROUND_OBJECT_ID (see $1645)
     * 
     * In level data, ground object types:
     * 1             Rocket
     * 2             Fuel tank
     * 4             Mystery
     * 8             Base 
     */
void addLandscapeCol(void){
  shiftTiles(-1,BLANK_TILE);
  for(uint8_t n=0;n<31;n++){
    gV.colAttr[n]=gV.colAttr[n+1];
  }
  for(uint8_t n=0;n<MAX_BULLETS;n++){
    --gV.bulletOb[n].pos;
  }
  const uint8_t *p=levelData+gV.levelAddr;
  uint8_t tileIX,tilePos;
  uint8_t sOff=gV.ff<<1;
  tilePos=pgm_read_byte(p+sOff);
  if(tilePos==255){ // Level change flag
    if(gV.level<5){
      ++gV.level;
    }
    setLevelMetadata();
    updateLevelLivesUI();
    p=levelData+gV.levelAddr;
    tilePos=pgm_read_byte(p+sOff);
  }

  tilePos=(tilePos>>3)+2;
  tileIX=pgm_read_byte(p+sOff+1);
  uint8_t cb=pgm_read_byte(p+4);
  uint8_t *dst=screenRam+(BYTES_PER_BUFFER_LINE*tilePos)+31;
  *dst=tileIX;
  gix=pgm_read_byte(p+(cb==0?5:8));
  gV.colAttr[31]=0;
  if(gix){
    gV.colAttr[31]=addGroundObject(gix,dst-(BYTES_PER_BUFFER_LINE*2));
  }

  while(tilePos<31){
    dst+=BYTES_PER_BUFFER_LINE;
    *dst=gV.fillBlock[gV.ff];
    ++tilePos;
  }

  if(cb==0){
    if(gV.ff){
      gV.levelAddr+=6;
      gV.ff=0;
    }else{
      gV.ff=1;
    }
  }else{
    tilePos=(pgm_read_byte(p+4+sOff)>>3)+2;
    tileIX=pgm_read_byte(p+4+sOff+1);
    dst=screenRam+(BYTES_PER_BUFFER_LINE*tilePos)+31;
    *dst=tileIX;
    while(tilePos>7){
      dst-=BYTES_PER_BUFFER_LINE;
      *dst=gV.fillBlock[gV.ff];
      --tilePos;
    }
    if(gV.ff){
      gV.levelAddr+=9;
      gV.ff=0;
    }else{
      gV.ff=1;
    }

  }
}

// Move bullets on screen and check for hits against landscape, ground structures and flying enemies
void updateBullets(void){
  uint8_t *p=screenRam;
  for(uint8_t n=0;n<MAX_BULLETS;n++){

    // Is bullet active?
    if(gV.bulletOb[n].tileIX>175 && gV.bulletOb[n].tileIX<192){

      // If bullet drawn, erase it
      if(*gV.bulletOb[n].pos>175 && *gV.bulletOb[n].pos<192){
        *gV.bulletOb[n].pos=BLANK_TILE;
      }

      // Move if animation frame to right of tileIX cells
      if(gV.bulletOb[n].tileIX>183){
        ++gV.bulletOb[n].pos;
        gV.bulletOb[n].tileIX-=8;
      }else{
        gV.bulletOb[n].tileIX+=8;
      }

      // Off end of screen?
      if( ((gV.bulletOb[n].pos-p)%32) > 29 ){
        gV.bulletOb[n].tileIX=0;
      }else{

        // If we blew up a structure, a flying alien, or hit non-destructible landscape, bullet dead
        gix=((gV.bulletOb[n].pos-p)%BYTES_PER_BUFFER_LINE);
        giy=(((gV.bulletOb[n].pos-p)/BYTES_PER_BUFFER_LINE)<<3)+(gV.bulletOb[n].tileIX&0x07)-16;

        if(
          explodeFromTile(gV.bulletOb[n].pos) || 
          explodeFromFlying(gix,giy,gV.bulletOb[n].tileIX)
        ){
          startAudio(1,bulletHitSound);
          gV.bulletOb[n].tileIX=0;
        }else if((*gV.bulletOb[n].pos<112 && *gV.bulletOb[n].pos!=BLANK_TILE)){
          gV.bulletOb[n].tileIX=0;
        // Can we draw it?
        }else if(*gV.bulletOb[n].pos==BLANK_TILE){
          *gV.bulletOb[n].pos=gV.bulletOb[n].tileIX;
        }

      }
      
    }
  }

}

uint8_t checkSpriteOverlap(uint8_t ix0,uint8_t ix1){

  ProcessedSprite *s0=processedSprite+ix0;
  ProcessedSprite *s1=processedSprite+ix1;

  for(uint8_t x=0;x<3;x++){ // byte position of player sprite

    uint8_t dByte=(s0->xByte+x)-s1->xByte;
    if(dByte>2){
      continue;
    }

    uint8_t sByte=x<<4;
            dByte<<=4;
    uint8_t yOff=s0->yLine-s1->yLine;

    for(uint8_t y=0;y<16;y++){
      uint8_t yDelta=y+yOff;
      if(yDelta<16){
        if( s0->sData[sByte] & s1->sData[dByte+yDelta] ){
          return 1;
        }
      }
      ++sByte;
    }
  }
  return 0;
}

uint8_t checkPlayerVSFlying(void){
  // Go through both player ship sprites, checking overlapping bytes from the flying alien sprites 
  for(uint8_t ix0=7;ix0<9;ix0++){

    uint8_t pY=processedSprite[ix0].yLine;
    uint8_t pXc=processedSprite[ix0].xByte;

    for(uint8_t ix1=0;ix1<MAX_FLYING_ENEMIES;ix1++){
      uint8_t sDef=spriteData[ix1].defIX;

      // If not a rocket, saucer or fireball, we don't care (it will be an explosion, currently)
      if( (sDef<13 || sDef>19) && (sDef!=33) && (sDef!=34) ){
        continue;
      }
      // If sprite is too far above or below, then we don't care
      if((processedSprite[ix1].yLine<pY-15) || (processedSprite[ix1].yLine>pY+15)){
        continue;
      }
      // If sprite is too far left or right, then we don't care
      if((processedSprite[ix1].xByte<pXc-2) || (processedSprite[ix1].xByte>pXc+1)){
        continue;
      }

      if(checkSpriteOverlap(ix0,ix1)){

          // Saucer? Add 100pts
          if(sDef<33){
            addScore(10);
          // Rocket? Add 80pts
          }else{
            addScore(8);
          }
          setSpriteDef(ix1,23);

        return 1;
      }
    }
  }
  return 0;
}

uint8_t explodeFromFlying(uint8_t charX,uint8_t startY, uint8_t checkByte){
  for(uint8_t n=0;n<MAX_FLYING_ENEMIES;n++){
    uint8_t sDef=spriteData[n].defIX;
    // Can only blow up flying rockets and saucers - fireballs are immune
    if(sDef!=13 && sDef!=14 && sDef!=15 && sDef!=33 && sDef!=34){
      continue;
    }
    uint8_t sY=startY-(processedSprite[n].yLine+1);
    uint8_t scX=charX-processedSprite[n].xByte;
    if( (sY>15) || (scX>2) ){
      continue;
    }
    uint8_t b=processedSprite[n].sData[(scX<<4)+sY];
    if((b&checkByte)==0){
      continue;
    }
    // Saucer? Add 100pts
    if(sDef<33){
      addScore(10);
    
    // Rocket? Add 80pts
    }else{
      addScore(8);
    }
    setSpriteDef(n,23);
    return 1;
  }
  return 0;
}

// Used by bombs and bullets to check for hits and blow up enemy structures on landscape
uint8_t explodeFromTile(uint8_t *p){

  // Ignore hitting own bullets or explosions on tilemap
  if((*p==BLANK_TILE) || (*p>139) || (*p<128 && *p>123)){
    return 0;
  }

  // Check if it's an enemy item on the landscape - if so, we need to
  // update the score and make it explode (or show points for Mystery)
  if((*p>=112 && *p<=123) || (*p>=128 && *p<=139)){

    // Check if we're on a bottom block for the structure, if so, go up a row
    if(*p>=128){
      p-=BYTES_PER_BUFFER_LINE;
    }

    // If an odd index, it's the right side of the structure.
    if((*p)&0x01){
      --p;
    }

    // Should now be at the top left corner of the structure that was hit. Find the type
    // to add it to the score, and make it explode or show its score value if Mystery
    uint8_t tix=146;
    uint8_t scoreAdd=0;
    switch(*p){
      // Rocket
      case 112:scoreAdd=5;break;
      // Fuel
      case 114:
        scoreAdd=15;
        gV.fuel+=24;
        if(gV.fuel>128){
          gV.fuel=128;
        }
        updateFuelUI();
        break;
      // Mystery
      case 116:
        tix=(currentFrame%3);  // 0,1, or 2
        scoreAdd=(tix*10)+10;      // 10,20 or 30
        switch(tix){
          case 0: tix=124;break;
          case 1: tix=126;break;
          case 2: tix=144;break;
        }
        break;
      // Base (has 3 animation frames)
      case 118:
      case 120:
      case 122:
        scoreAdd=80;
        SET_HIT_BASE;
        break;
    }

    *p=tix;
    *(p+1)=tix+1;
    *(p+BYTES_PER_BUFFER_LINE)=tix+16;
    *(p+BYTES_PER_BUFFER_LINE+1)=tix+17;
    gV.colAttr[(p-screenRam)%32]=tix;
    addScore(scoreAdd);
  }

  return 1;
}

void clearStructure(uint8_t *p){
  *p=BLANK_TILE;
  *(p+1)=BLANK_TILE;
  *(p+BYTES_PER_BUFFER_LINE)=BLANK_TILE;
  *(p+BYTES_PER_BUFFER_LINE+1)=BLANK_TILE;
}

uint8_t updateGroundAnimations(void){
  if(currentFrame%4){
    return 0;
  }
  uint8_t *p;
  giu=0;
  for(uint8_t n=0;n<31;n++){
    giz=gV.colAttr[n];
    if(
      ( (giz>=146) && (giz<=152) && ((currentFrame%8)==0) ) ||
      ( (giz>=118) && (giz<=122) && ((currentFrame%4)==0) )
    ){
      p=screenRam+(30*BYTES_PER_BUFFER_LINE)+n;
      for(uint8_t i=30;i>6;i--){
        if(*p==giz){
          giz+=2;
          if(giz==154){
            clearStructure(p);
            gV.colAttr[n]=0;
          }else{
            if(giz==124){
              giz=118;
            }
            *p=giz;
            *(p+1)=giz+1;
            *(p+BYTES_PER_BUFFER_LINE)=giz+16;
            *(p+BYTES_PER_BUFFER_LINE+1)=giz+16+1;
            gV.colAttr[n]=giz;
            giu=1;
          }
          break;
        }
        p-=BYTES_PER_BUFFER_LINE;
      }
    }
  }
  return giu;
}

void moveBombs(void){
  for(uint8_t n=0;n<2;n++){
    gix=spriteData[n+5].defIX;
    if(gix==255){
      continue;
    }
    // Normal animation frames for bomb
    if(gix>7 && gix<13){
      giy=pgm_read_byte(bombAnim+gV.bombTmr[n]);  // Default to increasing in X and Y axis
      setSpritePos(n+5, spriteData[n+5].x+((giy&1)?1:0), spriteData[n+5].y+((giy&2)?1:0) );
      if(giy&4){
        setSpriteDef(n+5,gix+1);
      }else if(giy&8){
        setSpriteDef(n+5,gix-1);
      }
      if(gV.bombTmr[n]<42){
        ++gV.bombTmr[n];
      }
      // See if bomb has hit something...
      gix=(spriteData[n+5].x+8+hScroll)>>3;
      giy=((spriteData[n+5].y+8)>>3)+2;
      uint8_t *p=screenRam+(BYTES_PER_BUFFER_LINE*giy)+gix;
      uint8_t testByte=192>>(spriteData[n+5].x&0x07);
      if(explodeFromTile(p) || explodeFromFlying(gix,(spriteData[n+5].y+8),testByte)){
        setSpriteDef(n+5,20);
        startAudio(1,bombExplodeSound);
        gV.bombTmr[n]=0;
      }

    // Explosion frames for bomb
    }else if(gix>19 && gix<24){
      setSpritePos(n+5,spriteData[n+5].x-1,spriteData[n+5].y);
      if((currentFrame%4)==0){
        if(++gV.bombTmr[n]==4){
          setSpritePos(n+5,255,((n+5)*16)+50);
          setSpriteDef(n+5,255);
          stopAudioChannel(1);
        }else{
          if(++gix==24){
            gix=20;
          }
          setSpriteDef(n+5,gix);
        }
      }
    }
  }
}

uint8_t getFreeEnemySprite(void){
  for(uint8_t n=0;n<MAX_FLYING_ENEMIES;n++){
    if(spriteData[n].defIX==255){
      return n;
    }
  }
  return 255;
}

void launchRocket(void){
  giy=getFreeEnemySprite();
  if(giy==255){
    return;
  }
  giz=255;
  gix=(gV.playerX>>3)+4+(currentFrame%12);
  for(uint8_t n=gix;n<24;n++){
    if(gV.colAttr[n]==112){ // 112 is the index of the top left of the rocket tileIX
      giz=n;
      break;
    }
  }
  if(giz==255){
    for(uint8_t n=gix;n>3;n--){
      if(gV.colAttr[n]==112){ // 112 is the index of the top left of the rocket tileIX
        giz=n;
        break;
      }
    }
  }
  if(giz==255){
    return;
  }

  uint8_t *p=screenRam+(30*BYTES_PER_BUFFER_LINE)+giz;
  for(uint8_t n=30;n>6;n--){
    if(*p==112){
      clearStructure(p);
      setSpriteDef(giy,33);
      setSpritePos(giy,(giz<<3)-hScroll,((n-2)<<3)-1);
      break;
    }
    p-=BYTES_PER_BUFFER_LINE;
  }
}

void launchSaucer(void){
  giy=getFreeEnemySprite();
  if(giy==255){
    return;
  }
  gV.flyingEnemy[giy].movementIX=0;
  setSpriteDef(giy,13);
  setSpritePos(giy,240,126+(currentFrame>>6));
}

void launchFireball(void){
  giy=getFreeEnemySprite();
  if(giy==255){
    return;
  }
  setSpriteDef(giy,16);
  gV.lastFireballY+=16+(((gV.lastFireballY>>1)*3)+currentFrame+giy+gV.score+gV.playerX+gV.playerY+gV.fuel+gV.autoScoreInc)%102;
  setSpritePos(giy,240,42+(gV.lastFireballY%130));
}

uint8_t checkPlayerVsLandscape(void){
  // Force an early pre-process of sprites used for player ship so that we can use the data to compare against the landscape
  // Processed sprite data is planar (optimised for rasterising), so bytes 0,16,32 are the top row, then 1,17,33 etc
  // Need to ignore tileIX's above 139, and also 124 to 127 so we don't collide with explosions :)
  for(uint8_t ix=0;ix<2;ix++){
    giy=spriteData[ix+7].defIX;
    setSpriteDef(ix+7,ix+40);
    processSprite(&processedSprite[ix+7], spriteDef, &spriteData[ix+7]);
    setSpriteDef(ix+7,giy);
  }
  for(uint8_t ix=0;ix<2;ix++){
    uint8_t sY=processedSprite[ix+7].yLine+1;
    uint8_t *screenPtr=screenRam+(BYTES_PER_BUFFER_LINE*((sY>>3)+2))+processedSprite[ix+7].xByte;

    for(uint8_t xByte=0;xByte<3;xByte++){
      uint8_t byteLine=(sY&0x07);
      uint8_t sIX=xByte<<4;
      uint8_t *p=screenPtr;
      uint8_t sByte,tByte;

      for(uint8_t yLine=0;yLine<16;yLine++){
        sByte=processedSprite[ix+7].sData[sIX];
        if(*p>139 || (*p>=124 && *p<=127)){
          tByte=0;
        }else{
          tByte=pgm_read_byte(currentTileData+(256*byteLine)+(*p));
        }
        // Collision?
        if(tByte&sByte){
          return 1;
        }
        if(++byteLine==8){
          byteLine=0;
          p+=BYTES_PER_BUFFER_LINE;
        }
        ++sIX;
      }
      ++screenPtr;
    }
  }
  return 0;
}

void runCollisionChecks(void){

  setPlayerPos();

  // Run the pre-processor on the flying alien sprites, so we can do pixel-based collision detection
  for(uint8_t n=0;n<MAX_FLYING_ENEMIES;n++){
    processSprite(&processedSprite[n], spriteDef, &spriteData[n]);
  }
  updateBullets();
  moveBombs();

  if(IS_PAUSED){
    clearBlock(0,2,32);
  }

  if( (NOT_PLAYER_DEAD) && (checkPlayerVsLandscape() || checkPlayerVSFlying()) ){
    if(NOT_PAUSED){
      SET_PLAYER_DEAD;
      startAudio(0,playerExplodeSound);
      setSpriteDef(5,24);
      setSpriteDef(6,25);
      setSpritePos(5,gV.playerX+8,gV.playerY);
      setSpritePos(6,gV.playerX+24,gV.playerY);
    }else{
      drawNumber(1,0,2);
    }
  }
}

void updateAudio(void){
  for(uint8_t n=0;n<2;n++){
    if(gV.audioPtr[n]!=NULL){
      uint8_t v=pgm_read_byte(gV.audioPtr[n]+gV.audioPos[n]);
      if(v==2){
        setPWMChannelFreq(n,random(170,250));
        continue;
      }
      ++gV.audioPos[n];
      // Value of 1=jump to note IX
      if(v==1){
        v=pgm_read_byte(gV.audioPtr[n]+gV.audioPos[n]);
        gV.audioPos[n]=v;
        v=pgm_read_byte(gV.audioPtr[n]+gV.audioPos[n]);

      // Value of 0=end audio stream
      }else if(v==0){
        stopAudioChannel(n);
        continue;
      }
      setPWMChannelFreq(n,v);
    }
  }
}

void startAudio(uint8_t channel,const uint8_t *ptr){
  if(NOT_DEMO_MODE){
    gV.audioPos[channel]=1;
    gV.audioPtr[channel]=ptr;
    setPWMChannelBaseSetting(channel,pgm_read_byte(ptr));
    setPWMChannel(channel,1);
  }
}

void stopAudioChannel(uint8_t ix){
  setPWMChannel(ix,0);
  gV.audioPos[ix]=0;
  gV.audioPtr[ix]=NULL;
}

void stopAllAudio(void){
  for(uint8_t n=0;n<2;n++){
    stopAudioChannel(n);
  }
}

void doPlaying(void){

  if(NOT_PLAYER_DEAD){

    if(IS_DEMO_MODE){

      if( (gV.autoScoreInc<128) && (gV.playerX<104)){
        ++gV.playerX;
      }else if( (gV.autoScoreInc>127) && (gV.playerX<24)){
        --gV.playerX;
      }

    }else{

      // Start button pauses game
      if(PRESSED_START){
        if(IS_PAUSED){
          CLEAR_IS_PAUSED;
          clearBlock(13,6,8);
        }else{
          SET_IS_PAUSED;
        }
      }

      if(IS_PAUSED){
        if((currentFrame%64)==0){
          clearBlock(13,6,8);
        }else if((currentFrame%32)==0){
          drawFixedText(pausedInverseTxt,255);
        }
        runCollisionChecks();
        return;
      }

      if(PRESSING_UP && (gV.playerY>41) && (gV.fuel>0)){
        --gV.playerY;
      }else if((PRESSING_DOWN || (gV.fuel==0)) && (gV.playerY<212)){
        ++gV.playerY;
      }

      if(PRESSING_LEFT && (gV.playerX>24)){
        --gV.playerX;
      }else if(PRESSING_RIGHT && (gV.playerX<104)){
        ++gV.playerX;
      }

      if(PRESSED_A && (gV.fireLockout==0)){
        gV.fireLockout=2;
        gix=gV.playerX>>3;
        giu=gV.playerY+1;
        giy=(giu>>3)+3;
        giz=((gV.playerX&0x07)>3)?8:0;
        for(uint8_t n=0;n<MAX_BULLETS;n++){
          if(gV.bulletOb[n].tileIX==0){
            gV.bulletOb[n].pos=screenRam+(BYTES_PER_BUFFER_LINE*giy)+gix+4;
            gV.bulletOb[n].tileIX=176+giz+(giu&0x07);
            break;
          }
        }
      }

      if(PRESSED_B && (gV.bombLockout==0)){
        gix=0;
        if(spriteData[5].defIX==255){
          gix=5;
        }else if(spriteData[6].defIX==255){
          gix=6;
        }
        if(gix>0){
          gV.bombLockout=16;
          gV.bombTmr[gix-5]=0;
          setSpriteDef(gix,8);
          setSpritePos(gix,gV.playerX+6,gV.playerY+4);
          startAudio(1,bombFallingSound);
        }
      }

      if(gV.fireLockout>0){
        --gV.fireLockout;
      }
      if(gV.bombLockout>0){
        --gV.bombLockout;
      }

    }

    ++gV.autoScoreInc;

    // Every 64 frames, add 10 pts to score
    if((gV.autoScoreInc&0x3f)==0x3f){
      addScore(1);
    }

    setScroll((hScroll+1)%8,0);
    if(hScroll==0){
      addLandscapeCol();
    }


  // Player is dead, so let the explosion play out while aliens etc carry on moving before restarting level
  }else{

    setPlayerPos();
    setSpriteDef(7,40); // Turn off the engine smoke
    selectTileBank( (currentFrame%32)>15 ? 0:1 );
    if((currentFrame%4)==0){
      setSpritePos(5,gV.playerX+8,gV.playerY);
      setSpritePos(6,gV.playerX+24,gV.playerY);
      setSpriteDef(5,pgm_read_byte(playerExplosionAnim+gV.playerAnimCounter-1));
      setSpriteDef(6,spriteData[5].defIX+1);
      if(++gV.playerAnimCounter>=24){
        if(gV.lives==0){
          SET_GAME_OVER;
        }else{
          SET_LOST_LIFE;
        }
      }
    }

  }

  runCollisionChecks();

  switch(gV.levelType){

    case 0:
    case 8:
    {
      // Rockets enabled
      // Every 64 frames, try to launch a rocket (preferably close to player, but otherwise any available - biased to ahead of the player)
      if((gV.autoScoreInc&0x3f)==0x3f){
        launchRocket();
      }

    }
    break;

    case 1:
    // Fireballs enabled
    // Every 16 frames, add a Fireball at a pseudo-random position in the vertical axis (based on player position, fuel, score, distance, currentFrame)
    if((gV.autoScoreInc&0x0f)==0x0f){
      launchFireball();
    }
    break;

    case 2:
    // Saucers enabled
    // Every 64 frames, add a saucer, always at same position, and moving in same figure of eight pattern
    if((gV.autoScoreInc&0x3f)==0x3f){
      launchSaucer();
    }

    break;

    case 4:
    // Nothing really - just the maze

    break;

    case 0x10:
    // Base - nothing special to do here, except end the current loop if base is destroyed

    break;

  }

  for(uint8_t n=0;n<MAX_FLYING_ENEMIES;n++){
    gix=spriteData[n].defIX;

    // Rocket?
    if(gix==33 || gix==34){
      setSpriteDef(n,33+((currentFrame>>2)%2));
      if(NOT_PLAYER_DEAD){
        --spriteData[n].x;
      }
      if(spriteData[n].x==0){
        resetSprite(n);
      }
      if(--spriteData[n].y<=40){
        resetSprite(n);
      }

    // Saucer?
    }else if(gix>=13 && gix<=15){
      spriteData[n].y-=pgm_read_byte(saucerAnim+gV.flyingEnemy[n].movementIX++);
      spriteData[n].x-=pgm_read_byte(saucerAnim+gV.flyingEnemy[n].movementIX++);
      if(gV.flyingEnemy[n].movementIX==148){
        gV.flyingEnemy[n].movementIX=0;
      }
      setSpriteDef(n,13+((currentFrame>>2)%3));
      if(spriteData[n].x<2 || spriteData[n].x>250){
        resetSprite(n);
      }
    
    // Fireball?
    }else if(gix>=16 && gix<=19){
      setSpriteDef(n,16+(((currentFrame+n)>>2)%4));
      spriteData[n].x-=((NOT_PLAYER_DEAD)?4:3);
      if(spriteData[n].x==0 || spriteData[n].x>251){
        resetSprite(n);
      }
    
    // Exploding?
    }else if(gix>=20 && gix<=23){
      if((NOT_PLAYER_DEAD) && --spriteData[n].x==0){
        resetSprite(n);
      }
      if((currentFrame%5)==0){
        ++spriteData[n].y;
      }
      if((currentFrame%10)==0){
        gix=spriteData[n].defIX-1;
        if(gix<20){
          resetSprite(n);
        }else{
          setSpriteDef(n,gix);
        }
      }
    }
  }

  if((currentFrame%gV.fuelDepletionRate)==0){
    if(gV.fuel>0){
      --gV.fuel;
      updateFuelUI();
    }
  }

  giz=updateGroundAnimations();
  if((giz==0) && IS_HIT_BASE && (hScroll==0)){
    setState(completedLoop);
  }

  if(IS_LOST_LIFE){
    setState(lostLife);
  }else if(IS_GAME_OVER){
    if(IS_DEMO_MODE){
      setState(demoEnd);
    }else{
      setState(gameOver);
    }
  }

}

void storeHighScores(void){
  for(uint8_t n=0;n<10;n++){
    EEPROM.write(n*2,hiScoreList[n]&0xff);
    EEPROM.write((n*2)+1,hiScoreList[n]>>8);
  }
}

void drawStarField(void){
  uint8_t *d=screenRam;
  for(uint16_t n=0;n<BYTES_PER_BUFFER_LINE*32;n++){
    if(screenRam[n]>191){
      screenRam[n]=BLANK_TILE;
    }
  }
  
  const uint8_t *s=starFieldDataPtr;
  d=screenRam+4;

  // The stars in the fixed region are pretty simple - never move...
  for(uint8_t y=0;y<7;y++){
    for(uint8_t x=0;x<28;x++){
      uint8_t tileIX=pgm_read_byte(s);
      if(tileIX<255 && *d==BLANK_TILE){
        *d=(tileIX+192);
      }
      ++s;
      ++d;
    }
  }

  // The stars in the scrolling region can shift across and down by up to 1 tile position...
  uint8_t yPos=(vScroll>>3);
  uint8_t yOff=vScroll&0x07;
  uint8_t xOff=hScroll&0x07;

  for(uint8_t y=0;y<32-7;y++){
    d=screenRam+(7*32)+(yPos*BYTES_PER_BUFFER_LINE)+4;
    for(uint8_t x=0;x<28;x++){
      uint8_t tileIX=pgm_read_byte(s);
      if(tileIX<255 && *d==BLANK_TILE){
        uint8_t tileDotX=tileIX&0x07;
        uint8_t tileDotY=tileIX>>3;
        uint8_t offAdd=0;

        tileDotX+=xOff;
        if(tileDotX>7){
          tileDotX-=8;
          offAdd=1;
        }

        tileDotY+=yOff;
        if(tileDotY>7){
          tileDotY-=8;
          offAdd=BYTES_PER_BUFFER_LINE;
        }

        if(*(d+offAdd)==BLANK_TILE){
          *(d+offAdd)=(((tileDotY<<3)+tileDotX)+192);
        }
      }
      ++s;
      ++d;
    }
    if(++yPos>24){
      yPos=0;
    }
  }

  if(++starFieldCounter==100){
    starFieldCounter=0;
    starFieldDataPtr+=896;
    if(starFieldDataPtr-starFieldData > (896*2)){
      starFieldDataPtr=starFieldData;
    }
  }
}

void setState(GameState nextState){
  
  switch(nextState){

    case gameInit:
    {
      gV.score=0;
      stopAllAudio();
     selectTileBank(2);
    }
    break;

    case title:
      stopAllAudio();
      selectTileBank(2);
      resetDisplay();
    break;

    case demo:
      gV.level=0;
      gV.loop=0;
      gV.lives=0;
      gV.fuel=128;
    break;

    case demoEnd:
      resetSprites();
    break;

    case chooseGameType:
      gV.score=0;
      gV.level=0;
      gV.loop=0;
      gV.fuelDepletionRate=SLOWEST_FUEL_DEPLETION_RATE;
      gV.lives=2;
      gV.fuel=128;
      CLEAR_DEMO_MODE;
      selectTileBank(0);
      resetDisplay();
    break;

    case playPlayer:
      //updateLevelLivesUI();
    break;

    case playing:
      resetLevel();
      if(NOT_DEMO_MODE){
        updateLevelLivesUI();
      }
      updateFuelUI();
      clearBlock(11,20,10);
    break;

    case completedLoop:
      stopAllAudio();
      resetSprites();
    break;

    case lostLife:
      stopAllAudio();
      resetSprites();
      selectTileBank(0);
      --gV.lives;
    break;

    case gameOver:
      stopAllAudio();
      resetSprites();
      selectTileBank(0);
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

  // Uncomment to get some idea of spare time within frame. Usually over 7K if not doing much.
  //clearBlock(0,2,20);
  //drawNumber(waitFrames,0,2);

  // Update the state of the controller/button inputs
  readInput();

  // This is the core of the game - we decide what to do based on the current "state" of the game. In this case
  // we are always in the "gameInit" state, and so just animate the sprites and scroll the tiles constantly.
  switch(gameState){
    
    case gameInit:
    {
      if(giu==0){
        if(++gix==1){
          resetSprites();
          clearTileMap(47);
          setScroll(0,0);
        } else if(gix==150){
          giu=1;
          gix=0;
        }
      }else if(giu==1){
        clearCol(31-gix,0,32);
        if(++gix==32){
          giu=2;
          gix=0;
        }
      }else if(giu==2){
        giz=0;
        for(uint8_t n=0;n<10;n++){
          hiScoreList[n]=(((uint16_t)EEPROM.read((n*2)+1))<<8) + (uint16_t)EEPROM.read(n*2);
          if(hiScoreList[n]==65535){
            giz=1;
          }
        }
        resetDisplay();
        if(PRESSING_START || giz==1){
          for(uint8_t n=0;n<10;n++){
            hiScoreList[n]=1000;
          }
          storeHighScores();
          resetDisplay();
          drawFixedText(hiScoresResetTxt,255);
          giu=3;
        }else{
          setState(title);
        }
      }else if(giu==3){
        if(++gix==100){
          setState(title);
        }
      }
    }
    break;

    case title:
    {
      doTitleAnimation();
      if(credits>0){
        setState(chooseGameType);
      }
    }
    break;

    case demo:
      if(giu==0){
        clearCol(31-gix,7,25);
        if(++gix==32){
          giu=1;
          gix=0;
        }
      }else{
        setScroll(0,0);
        selectTileBank(0);
        SET_DEMO_MODE;
        resetLevel();
        updateLevelLivesUI();
        setState(playing);
      }
    break;

    case demoEnd:
      if(giu==0){
        clearCol(31-gix,7,25);
        if(++gix==32){
          giu=1;
          gix=0;
        }
      }else{
        setState(title);
      }
    break;

    case chooseGameType:
    {
      doStartAnimation();
    }
    break;
    
    case playPlayer:
    {
      doPlayPlayerAnimation();
    }
    break;

    case playing:
    {
      doPlaying();
      if(IS_DEMO_MODE){
        if(credits>0){
          setState(chooseGameType);
        }
      }
    }
    break;

    case completedLoop:
    {
      doCompletedAnimation();
    }
    break;

    case lostLife:
    {
      if(giu==0){
        clearCol(31-gix,6,26);
        if(++gix==32){
          giu=1;
        }
      }else if(giu==1){
        updateLevelLivesUI();
        CLEAR_LOST_LIFE;
        setState(playing);
      }
    }
    break;
        
    case gameOver:
    {
      if(giu==0){
        resetSprites();
        clearCol(31-gix,6,26);
        if(++gix==32){
          for(uint8_t n=0;n<10;n++){
            if(gV.score>hiScoreList[n]){
              for(uint8_t i=9;i>n;i--){
                hiScoreList[i]=hiScoreList[i-1];
              }
              hiScoreList[n]=gV.score;
              storeHighScores();
              break;
            }
          }
          giu=1;
          gix=0;
          drawFixedText(playerOneTxt,255);
          drawFixedText(gameOverTxt,255);
        }
      }else if(giu==1){
        if(++gix==100){
          setState(title);
        }
      }
    }
    break;
  }

  if(NOT_DEMO_MODE && (gameState==playPlayer || gameState==playing || gameState==lostLife || gameState==gameOver || gameState==completedLoop)){
    if((currentFrame&0x0f)==0){
      if(screenRam[6]==BLANK_TILE){
        drawFixedText(scoreLineTxt, 255);
      }else{
        drawChar(BLANK_TILE,6,0);
        drawChar(BLANK_TILE,7,0);
        drawChar(BLANK_TILE,8,0);
      }
    }
  }

  // Accept credits at any time, up to 99
  if(PRESSED_OPTION && credits<99){
    ++credits;
    if(gameState==title || gameState==chooseGameType || IS_DEMO_MODE){
      drawCredits();
    }
  }

  updateAudio();

  // Always clear and redraw the stars last...
  drawStarField();

  // Update the sprite data ready for rendering in the next frame
  processSprites();

  // The current user code will now pause until the display is redrawn
}

#endif