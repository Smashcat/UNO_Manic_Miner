#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Space Invaders\\SpaceInvaders\\game.h"
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

extern "C" { void storeHi(); }

#include "gameDefs.h"
#include "engine.h"
#include "gameVars.h"
#include "gameFuncs.h"

void drawInvader(uint8_t ix, uint8_t col, uint8_t row){
  int8_t bitTest=bitAt(ix);
  if(bitTest==NO_INVADER){
    return;
  }

  int8_t pixOff=gameData.invaderOffset;
  int8_t cX=gameData.cursorX;
  if(ix>gameData.invaderUpdateBit){
    if(gameData.invaderDir>0){
      pixOff-=2;
      if(pixOff<0){
        pixOff=6;
        --cX;
      }
    }else{
      pixOff+=2;
      if(pixOff>6){
        pixOff=0;
        ++cX;
      }
    }
  }

  uint8_t cY=gameData.cursorY;
  if(INVADERS_DROPPING){
    if(ix<=gameData.invaderUpdateBit){
      if(ix==gameData.invaderUpdateBit){
        bitTest&=~INVADER_RIGHT;
      }
      ++cY;
    }else if(ix==gameData.invaderUpdateBit+1){
      bitTest&=~INVADER_LEFT;
    }
  }

  uint8_t aOff=(pixOff>>1)*3;
  uint16_t refIX=(120*(row/2))+aOff;
  refIX+=(bitTest*12);
  uint8_t tileIX0=pgm_read_byte(animLookup+refIX);
  uint8_t tileIX1=pgm_read_byte(animLookup+refIX+1);
  uint8_t tileIX2=pgm_read_byte(animLookup+refIX+2);
  uint8_t xPos=cX+(col*2);
  uint8_t yPos=(cY-(row*2));
  uint8_t *p=screenRam+(xPos+(yPos*BYTES_PER_BUFFER_LINE));
  *p++=tileIX0;
  *p++=tileIX1;
  *p++=tileIX2;

  // Invaders can destroy shields by walking through them
  if(yPos==24 || yPos==25){
    uint8_t sLRowIX=(yPos-24)*(16*8);
    for(uint8_t n=0;n<2;n++){
      uint8_t tIX=pgm_read_byte(shieldTile+xPos+n);
      if(tIX<255){
        uint8_t ptr=(sLRowIX+tIX);
        for(uint8_t i=0;i<8;i++){
          shieldLineBuffer[ptr+(i*16)]=0;
        }
      }
    }
  }else if(yPos==27 && (!PLAYER_EXPLODING)){
    SET_ALIENS_LANDED;
    SET_PLAYER_EXPLODING;
    gameData.playerTmr=0;
  }

}

// We redraw all active invaders every frame. Just makes it easier, 
// and also handles changing players with the same function.
void drawInvaders(){
  uint8_t cY=gameData.cursorY;
  if(cY>26){
    cY=26;
  }
  uint8_t *p=screenRam+((cY-8)*BYTES_PER_BUFFER_LINE);
  for(uint16_t n=0;n<10*BYTES_PER_BUFFER_LINE;n++){ // Draw 10 rows, to also handle dropping invaders...
    *p++=blankTile;
  }

  if(!INVADERS_PAUSED){
    if(!incUpdateBit()){
      return;
    }
  }

  uint8_t ix=0;
  for(uint8_t row=0;row<INVADER_ROWS;row++){
    for(uint8_t col=0;col<INVADERS_PER_ROW;col++){
      if(ix<gameData.revealedInvaders){
        drawInvader(ix++,col,row);
      }
    }
  }


  if(gameData.revealedInvaders==NUM_INVADERS && (!INVADERS_PAUSED)){
    // If entire rank has updated, then change offset to "ripple" across screen...
    if(isLastInvaderToUpdate()){
      if(INVADERS_DROPPING){
        UNSET_INVADERS_DROPPING;
        ++gameData.cursorY;
      }
      gameData.invaderOffset+=gameData.invaderDir;
      if(gameData.invaderOffset==8){
        gameData.invaderOffset=0;
        if(gameData.cursorX>=gameData.cursorXMax){
          gameData.invaderDir=-2;
          gameData.invaderOffset=4;
          SET_INVADERS_DROPPING;
        }else{
          ++gameData.cursorX;
        }
      }else if(gameData.invaderOffset<0){
        gameData.invaderOffset=6;
        if(gameData.cursorX<=gameData.cursorXMin){
          gameData.invaderDir=2;
          gameData.invaderOffset=2;
          SET_INVADERS_DROPPING;
        }else{
          --gameData.cursorX;
        }
      }
    }
  }


}

void checkInvaderHit(){
  SpriteData *s=spriteData+0;
  uint8_t cyP=gameData.cursorY*8;

  // Above or below rank, so skip out
  if((s->y>cyP+8) || (s->y<cyP-(8*8))){
    return;
  }

  uint8_t xC=(s->x>>3);
  uint8_t yC=(s->y>>3);
  uint8_t tileIX=screenRam[xC+(yC*BYTES_PER_BUFFER_LINE)];
  if((tileIX==2) || (tileIX==blankTile)){
    return;
  }

  // Now check for pixel collision in the 1x4 pixel player missile sprite
  uint8_t bitTest=(0x80>>(s->x&0x07));
  uint8_t gotIt=0;
  const uint8_t *src=tileData+tileIX;
  uint16_t offset=(s->y & 0x07);
  offset<<=8;
  src+=offset;
  for(uint8_t n=0;n<4;n++){
    uint8_t pgmByte=pgm_read_byte(src);
    if(pgmByte&bitTest){
      gotIt=1;
      break;
    }
    src+=256;
  }

  if(!gotIt){
    return;
  }

  int16_t spriteX=s->x;
  uint8_t bitRow=((gameData.cursorY-yC)+1)>>1;
  int16_t cxP=(gameData.cursorX*8)+gameData.invaderOffset;
  int8_t relPos=(spriteX-cxP)>>4;

  gameData.missile[0].expTmr=pgm_read_byte(&bombExplodeFrames);
  uint8_t hasPix=(gameData.invaderBitmap[bitRow]&(1<<relPos))?1:0;
  gameData.invaderBitmap[bitRow]&=~(1<<relPos);
  getInvaderMinMax();
  setSpriteDef(0,13);
  setSpritePos(0,cxP+2+(relPos<<4),(s->y&0xf8));
  addScore((bitRow>>1)+1);
  setReloadRate();
  if(gameData.remainingInvaders>0){
    --gameData.remainingInvaders;
    if(gameData.remainingInvaders<9){
      gameData.alienMissileSpeed=5;
      gameData.saucerTmr=9999;
    }
  }
  const uint8_t beatDelay=pgm_read_byte(audioDelayTable+gameData.beatDelayIX);
  if(gameData.remainingInvaders<beatDelay && gameData.beatDelayIX<16){
    ++gameData.beatDelayIX;
  }
  SET_INVADERS_PAUSED;
}

void updateSaucer(){
  if(!SAUCER_ACTIVE){
    --gameData.saucerTmr;
    if(gameData.saucerTmr==0){
      SET_SAUCER_ACTIVE;
      setPWMChannelBaseSetting(1, FREQ64  );
      if(gameData.shotCnt%2){ // odd=left
        gameData.saucerX=216;
        gameData.saucerDir=-2;
      }else{
        gameData.saucerX=16;
        gameData.saucerDir=2;
      }
    }
    return;
  }

  clearBufferLine(5);
  if(SAUCER_EXPLODING){
    if(++gameData.saucerTmr<20){
      uint8_t tix=44+((gameData.saucerX&0x07)<<1);
      uint8_t pix=(gameData.saucerX>>3);
      uint8_t *p=screenRam+(pix+(5*BYTES_PER_BUFFER_LINE));
      *p++=tix++;
      *p++=tix++;
      *p++=tix++;
      *p++=tix++;
      setFreq(1,((gameData.saucerTmr&0x03)*8)+170);
    }else if(gameData.saucerTmr<32){
      setFreq(1,((gameData.saucerTmr&0x03)*8)+170);
      drawNumber((gameData.saucerX>>3),5,gameData.saucerScore*10,gameData.saucerScore<10?2:3);
    }else{
      reloadSaucerFrames();
      UNSET_SAUCER_ACTIVE;
      UNSET_SAUCER_EXPLODING;
      setFreq(1,0);
      setPWMChannelBaseSetting(1, FREQ64  );
    }
  }else{
    gameData.saucerX+=gameData.saucerDir;
    setFreq(1,(((gameData.saucerX>>1)&0x03)*10)+200);
    if(gameData.saucerX<16 || gameData.saucerX>216){
      reloadSaucerFrames();
      UNSET_SAUCER_ACTIVE;
      setFreq(1,0);
      setPWMChannelBaseSetting(1, FREQ64  );
    }else{
      uint8_t tix=32+(((gameData.saucerX&0x07)*3)>>1);
      uint8_t pix=(gameData.saucerX>>3);
      uint8_t *p=screenRam+(pix+(5*BYTES_PER_BUFFER_LINE));
      *p++=tix++;
      *p++=tix++;
      *p++=tix++;
    }
  }
}

int8_t lowestInvaderForCol(int8_t tryColIX){
  const uint16_t bitPos=(((uint16_t)1)<<tryColIX);
  for(int8_t n=0;n<INVADER_ROWS;n++){
    if(gameData.invaderBitmap[n]&bitPos){
      return n;
    }
  }
  return -1;
}

void getBombPosByPlayerPos(uint8_t ix, int8_t &col, int8_t &invaderRow){
  const uint8_t pX=gameData.playerX;
  uint8_t rX=(gameData.cursorX*8)+gameData.invaderOffset+7;
  uint8_t minDist=255;
  col=-1;
  for(uint8_t n=0;n<INVADERS_PER_ROW;n++){
    const int8_t i=lowestInvaderForCol(n);
    if(i>-1){
      const uint8_t dist=((rX>pX)?rX-pX:pX-rX);
      if(dist<minDist){
        minDist=dist;
        col=n;
        invaderRow=i;
      }
    }
    rX+=16;
  }
}

void getBombPosBySequence(uint8_t ix, int8_t &col, int8_t &invaderRow){
  Missile *m=gameData.missile+ix;
  const uint8_t lowIX=pgm_read_byte(bombColRange+((ix-1)*2));
  const uint8_t highIX=pgm_read_byte(bombColRange+1+((ix-1)*2));
  col=-1;
  invaderRow=-1;
  for(uint8_t n=0;n<15;n++){
    int8_t tryColIX=pgm_read_byte(bombCols+m->colIX)-1;
    int8_t i=lowestInvaderForCol(tryColIX);
    if(++m->colIX>highIX){
      m->colIX=lowIX;
    }
    if(i>-1){
      col=tryColIX;
      invaderRow=i;
      return;
    }
  }
}

void shootMissile(uint8_t ix){
  // No new missiles can be shot whilst player is exploding
  if(PLAYER_EXPLODING){
    return;
  }
  Missile *m=gameData.missile+ix;
  // Missile still active, cannot re-shoot
  if(m->dirY!=0){
    return;
  }
  int dirY;
  uint8_t xPos,yPos;

  // Missile 0 is player's shot
  if(ix==0){
    xPos=gameData.playerX+8;
    yPos=212;
    dirY=-4;
    if(++gameData.shotCnt==30){
      gameData.shotCnt=0;
    }
  // Others are alien bombs
  }else{

    // Too soon or not this bomb's "turn" in the 0-2 counter
    if(INVADERS_PAUSED || (gameData.movTmr>0) || (ix!=(gameData.bombIX+1))){
      return;
    }

    int8_t col=-1;
    int8_t invaderRow=-1;
    // Missile 1 and 2 use a sequence of columns, if a column has no invaders, it tries the next in the sequence. 
    // If none found from sequence (not all columns are available to both bombs) then it uses the same method as 
    // missile 3 (rolling), and chooses the column closest to the player X position
    if(ix<3){
       getBombPosBySequence(ix,col,invaderRow);
       if(col==-1){
        getBombPosByPlayerPos(ix,col,invaderRow);
       }
    // Missile 3 is rolling, which targets player. Others use specific columns
    }else{
      getBombPosByPlayerPos(ix,col,invaderRow);
    }

    if(col==-1){
      return;
    }

    xPos=(gameData.cursorX*8)+gameData.invaderOffset+(col*16)+7;  // The +7 just centres the bomb sprite under the alien
    yPos=(gameData.cursorY-(invaderRow*2)+2)*8;
    dirY=gameData.alienMissileSpeed;
    gameData.movTmr=gameData.reloadRate;
  }

  m->expTmr=0;
  m->dirY=dirY;
  setSpriteDef(ix,m->baseDefIX);
  setSpritePos(ix,xPos,yPos);
}

void drawPlayer(){
  uint8_t playerTileBase=60;
  if(SHOW_PLAYER){
    if(!PLAYER_EXPLODING){
      clearBufferLine(27);
      if(DEMO_MODE){
        if(gameData.demoModeTmr==0){
          gameData.demoModePlayerTargetX=pgm_read_byte(playerLimitX+0)+16+(currentFrame%170);
          gameData.demoModeTmr=(gameData.demoModePlayerTargetX>gameData.playerX?gameData.demoModePlayerTargetX-gameData.playerX:gameData.playerX-gameData.demoModePlayerTargetX);
          gameData.demoModeTmr+=(currentFrame%32);
        }else{
          --gameData.demoModeTmr;
          if(gameData.playerX<gameData.demoModePlayerTargetX){
            ++gameData.playerX;
          }else if(gameData.playerX>gameData.demoModePlayerTargetX){
            --gameData.playerX;
          }
        }
        shootMissile(0); // In demo mode player constantly attempts to shoot
      }else{
        if(PRESSING_LEFT & gameData.playerX>pgm_read_byte(playerLimitX+0)+16){
          --gameData.playerX;
        }else if(PRESSING_RIGHT & gameData.playerX<pgm_read_byte(playerLimitX+1)+16){
          ++gameData.playerX;
        }
        if(PRESSING_A && (!FIRE_LOCKED_OUT)){
          shootMissile(0);
          SET_FIRE_LOCKED_OUT;
        }else{
          UNSET_FIRE_LOCKED_OUT;
        }
      }
    }else{
      playerTileBase=(((currentFrame>>2)&0x01)?60+24:60+24+24);
      if(++gameData.playerTmr==64){
        SET_PLAYER_DEAD;
      }
    }

    uint8_t tix=playerTileBase+((gameData.playerX&0x07)*3);
    uint8_t pix=(gameData.playerX>>3);
    uint8_t *p=screenRam+(pix+(27*BYTES_PER_BUFFER_LINE));
    *p++=tix++;
    *p++=tix++;
    *p++=tix++;
  }else{
    clearBufferLine(27);
  }
}

void explodeMissile(uint8_t ix){
  gameData.missile[ix].expTmr=pgm_read_byte(ix==0?&bombExplodeFrames:&bombExplodeFrames);
  setSpriteDef(ix,gameData.missile[ix].expDefIX);
  setSpritePos(ix,spriteData[ix].x-(ix==0?4:3),spriteData[ix].y);
}

void testSpriteShieldCollisions(){
  const uint8_t shieldYPix=(25*8);
  for(uint8_t n=0;n<NUM_MISSILES;n++){
    ProcessedSprite pS=processedSprite[n];

    // First check if sprite is in line with a shield
    uint8_t shieldTileRef=pgm_read_byte(shieldTile+pS.xByte);
    if(shieldTileRef==255){
      continue;
    }

    int8_t lineOff=( pS.yLine - (shieldYPix-7) );
    // There are 23 lines of possible collision between an 8 pixel high missile/bomb, and 16 pixel high shield
    if(lineOff<-7 || lineOff>15){
      continue;
    }

    uint8_t needsToChangeDef=(spriteData[n].defIX!=gameData.missile[n].expDefIX)?1:0;
    for(uint8_t i=0;i<8;i++){
      if(lineOff>-1 && lineOff<16){
        for(uint8_t t=0;t<2;t++){
          const uint8_t spByte=pS.sData[i+(t<<4)];
          const uint8_t shieldBufferIX=(lineOff<<4)+shieldTileRef+t;
          uint8_t slbByte=shieldLineBuffer[shieldBufferIX];
          if(spByte&slbByte){
            slbByte&=~spByte;
            shieldLineBuffer[shieldBufferIX]=slbByte;
            if(needsToChangeDef){
              explodeMissile(n);
              goto breakOut;  // short circuit the rest, we need to allow the ProcessedSprite data to update on the next frame
            }
          }
        }
      }
      ++lineOff;
    }
breakOut:;
  }
}

void drawMissiles(){
  for(uint8_t n=0;n<NUM_MISSILES;n++){
    if(gameData.missile[n].dirY==0){
      continue;
    }
    if(gameData.missile[n].expTmr==0){

      // Player missile moves every frame
      if(n==0){

        if(spriteData[n].y>32){
          spriteData[n].y+=gameData.missile[n].dirY;
          if(SAUCER_ACTIVE && (!SAUCER_EXPLODING) && (spriteData[n].y<=40)){
            if( (spriteData[n].x>=gameData.saucerX) && (spriteData[n].x<gameData.saucerX+16) ){
              gameData.saucerScore=pgm_read_byte(saucerScore+(gameData.shotCnt%15));
              addScore(gameData.saucerScore);
              SET_SAUCER_EXPLODING;

              gameData.missile[n].dirY=0;
              setSpritePos(n,255,n*20);
            }
          }
          checkInvaderHit();
          for(uint8_t i=1;i<NUM_MISSILES;i++){
            if((spriteData[i].x<=spriteData[0].x+1) && (spriteData[i].x>=spriteData[0].x-1) && (spriteData[i].y>=spriteData[0].y-3) && (spriteData[i].y<=spriteData[0].y+3)){
              explodeMissile(n);
              break;
            }
          }
        // Hit top
        }else{
          explodeMissile(n);
        }

      // Alien missiles take turns, so each moves every third frame
      }else if(n==(gameData.bombIX+1)){
        if(spriteData[n].y<228){
          spriteData[n].y+=gameData.missile[n].dirY;
          if(++spriteData[n].defIX==gameData.missile[n].baseDefIX+4){
            spriteData[n].defIX=gameData.missile[n].baseDefIX;
          }
#ifndef PLAYER_IMMUNE

          if(!PLAYER_EXPLODING){
            if( (spriteData[n].y>=212) && (spriteData[n].y<=220) && (spriteData[n].x>=(gameData.playerX+2)) && (spriteData[n].x<(gameData.playerX+14)) ){
              SET_PLAYER_EXPLODING;
              gameData.playerTmr=0;
              gameData.missile[n].dirY=0;
              setSpritePos(n,255,n*20);
            }
          }
#endif
        }else{
          explodeMissile(n);
        }
      }
    }else{
      if(--gameData.missile[n].expTmr==0){
        gameData.missile[n].dirY=0;
        setSpritePos(n,255,n*20);
      }
    }
  }
}

void setFreq(const uint8_t chan, uint8_t freq){
  #ifndef ENABLE_AUDIO_IN_DEMO_MODE
  if(DEMO_MODE){
    freq=0;
  }

  #endif
  setPWMChannelFreq(chan,freq);
}

void updateGame(){

  ++gameData.gameTmr;

    // This is ridiculous, but this is required to stop Arduino from "optimising out" 
    // the processedSprite array at index 3. Guessing because it's only populated by
    // the ASM code, but who knows why...
    for(uint8_t s=0;s<4;s++){
      for(uint8_t n=0;n<48;n++){
        processedSprite[s].sData[n]=255;
      }
    }

  if((!SHOW_PLAYER) && gameData.gameTmr==129){
    if(DEMO_MODE){
      gameData.saucerTmr=60;  // In demo mode, we show the saucer pretty quickly...
      gameData.reloadRate=7;  // Also set fastest reload rate
    }
    SET_SHOW_PLAYER;
    gameData.playerX=pgm_read_byte(playerLimitX+0)+16;
    setPWMChannelBaseSetting(0, FREQ128  );
    setPWMChannelBaseSetting(1, FREQ64  );
  }

  if(SHOW_PLAYER){

    if(!PLAYER_EXPLODING && !PLAYER_DEAD){
      if(gameData.beatTmr==0){
        gameData.beatTmr=pgm_read_byte(audioDelayTable+gameData.beatDelayIX+16);
        const uint8_t freq=pgm_read_byte(beats+gameData.beatIX);
        setFreq(0,freq);
        if(++gameData.beatIX==4){
          gameData.beatIX=0;
        }
        gameData.beatCutTmr=10;
      }
      if(gameData.beatTmr>0){
        --gameData.beatTmr;
      }
      if(gameData.beatCutTmr>0){
        if(--gameData.beatCutTmr==0){
          setFreq(0,0);
        }
      }
    }else if(PLAYER_EXPLODING){
      setPWMChannelBaseSetting(1, FREQ1024  );
      giz*=81+currentFrame;
      giz^=gameData.playerX;
      giz^=gameData.remainingInvaders;
      setFreq(0,(giz&0x1f)+90);
      giz*=73;
      giz>>=1;
      setFreq(1,(giz&0x1f)+90);
    }else if(PLAYER_DEAD){
      setPWMChannelBaseSetting(1, FREQ64  );
      setFreq(0,0);
      setFreq(1,0);
    }


    if(++gameData.bombIX==3){
      gameData.bombIX=0;
      if(gameData.movTmr>0){
        --gameData.movTmr;
      }
    }

    if(gameData.bombIX==0){
      updateSaucer();
    }
  }

  if(INVADERS_PAUSED){
    if(--gameData.invaderPauseTmr==0){
      UNSET_INVADERS_PAUSED;
      if(!SAUCER_ACTIVE){
        setFreq(1,0);
      }
    }else{
      if(!SAUCER_ACTIVE){
        setFreq(1,(gameData.invaderPauseTmr*10)+100);
      }
    }

  }else if(gameData.remainingInvaders==0){
    setState(levelClear);
  }

  if(!PLAYER_EXPLODING){
    drawInvaders();
  }


  drawPlayer();
  shootMissile(gameData.bombIX+1);  
  processSprites();
  testSpriteShieldCollisions();
  drawMissiles();
}

void setState(GameState nextState){
  
  switch(nextState){
    case gameInit:
      resetMachine();
    break;
    case title:
      resetGameScreen();
    break;
    case demo:
      resetGame();
      SET_DEMO_MODE;
    break;
    case pricing:
      UNSET_DEMO_MODE;
      setFreq(0,0);
      setFreq(1,0);
      resetGameScreen();
    break;
    case chooseGameType:
      UNSET_DEMO_MODE;
      resetGameScreen();
      resetGame();
    break;
    case playPlayer:
      resetGameScreen();
      resetInvaders();
    break;
    case playing:
    break;
    case levelClear:
    setFreq(0,0);
    setFreq(1,0);
    hideSprites();
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
  waitFrame();
  //drawNumber(0,30,waitFrames,5);
  readInput();
  switch(gameState){
    
    case gameInit:
      if(giu==0){
        resetGameScreen();
        resetInvaders();
        if(PRESSING_OPTION && PRESSING_START){
          hiScore=0;
          storeHi();
          giu=1;
        }else{
          setState(title);
        }
      }else if(giu==1){
        if(wipeScreen()){
          drawGameText(hiScoreResetTxt);
          giu=2;
          gix=0;
        }
      }else if(giu==2){
        if( (++gix>150) && (!PRESSING_OPTION && !PRESSING_START) ){
          gameData.credit=0;
          setState(title);
        }
      }
    break;
    
    case title:
      doTitleAnim();
    break;

    case demo:
      runDemo();
    break;

    case pricing:
      doPricingAnim();
    break;

    case chooseGameType:
      if(giu==0){
        if(wipeScreen()){
          giu=1;
          drawGameText(pushTxt);
        }
      }else if(giu==1){
        if(gameData.credit==1){
          drawGameText(onlyOnePlayerButtonTxt);
        }else{
          drawGameText(oneOrTwoPlayersButtonTxt);
        }
      }
      if(PRESSING_START){
        --gameData.credit;
        drawCredits();
        setState(playPlayer);
      }
    break;
    
    case playPlayer:
      if(giu==0){
        if(wipeScreen()){
          giu=1;
          drawGameText(playPlayerTxt);
          drawNumber(21,14,gameData.currentPlayerIX+1,1);
        }
      }else if(giu==1){
        if((++gix>>1)&1){
          clearBlock((gameData.currentPlayerIX==0?5:23),3,4);
        }else{
          drawScoreLine();
        }
        if(gix==200){
          setState(playing);
        }
      }
    break;

    case playing:
      if(giu==0){
        if(wipeScreen(CHARACTER_ROWS-3)){
          giu=1;
          gameData.invaderUpdateBit=0;
          drawLives();
        }
      }else if(giu==1){
        if(revealInvaders()){
          giu=2;
        }
      }else if(giu==2){
        if(PLAYER_DEAD){
          setFreq(0,0);
          setFreq(1,0);
          setPWMChannelBaseSetting(1, FREQ64  );
          clearBufferLine(27);
          resetMissiles();
          if(ALIENS_LANDED){
            gameData.lives[gameData.currentPlayerIX]=1;
          }
          --gameData.lives[gameData.currentPlayerIX];
          drawLives();
          if(gameData.lives[gameData.currentPlayerIX]>0){
            gix=0;
            giu=3;
          }else{
            setState(gameOver);
          }
        }
      }else if(giu==3){
        if(++gix==100){
          UNSET_PLAYER_DEAD;
          UNSET_PLAYER_EXPLODING;
          gameData.playerX=pgm_read_byte(playerLimitX+0)+16;
          giu=2;
        }
      }
      if(giu>0 && (!PLAYER_DEAD)){
        updateGame();
      }
    break;

    case levelClear:
      if(++gix==50){
        if(++gameData.level[gameData.currentPlayerIX]>8){
          gameData.level[gameData.currentPlayerIX]=0;
        }
        resetGameScreen();
        resetInvaders();
        setState(playing);
      }
    break;
        
    case gameOver:
      if(giu==0){
        if(drawSlowText(gameOverTxt)){
          giu=1;
          if(NEW_HI){
            storeHi();
            UNSET_NEW_HI;
          }
        }
      }else if(giu==1){
        if(++gix==100){
          setState(gameData.credit>0?chooseGameType:pricing);
        }
      }
    break;
  }

  // Allow adding credit at any time...
  if(PRESSING_OPTION){
    if(!OPTION_LOCKED_OUT){
      if(gameData.credit<99){
        ++gameData.credit;
        drawCredits();
      }
      if(gameState==demo || gameState==pricing || gameState==title){
        setState(chooseGameType);
      }
      SET_OPTION_LOCKED_OUT;
    }
  }else{
    UNSET_OPTION_LOCKED_OUT;
  }
}
