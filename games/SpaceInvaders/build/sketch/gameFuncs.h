#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Space Invaders\\SpaceInvaders\\gameFuncs.h"
#ifndef __GAME_FUNCS
#define __GAME_FUNCS

#include "gameDefs.h"
#include "engine.h"
#include "gameVars.h"



// Declarations

void drawNumber(const uint8_t x, const uint8_t y, uint16_t num, const uint8_t digits);
void drawBits(uint8_t x, uint8_t y, uint16_t num);
void drawText(uint8_t x, uint8_t y, const uint8_t *txt, uint8_t len);
uint8_t drawSlowText(const uint8_t *txt);
void drawAdvanceText();
void hideSprites();
void drawCredits();
void addScore(uint8_t s);
void drawScoreLine();
uint8_t wipeScreen(uint8_t numRows=CHARACTER_ROWS);
void doTitleAnim();
void initShields();
void clearShields();
int8_t bitAt(uint8_t bitPos);
uint8_t isLastInvaderToUpdate();
uint8_t incUpdateBit();
uint8_t revealInvaders();
void resetMachine();
void setReloadRate();
void clearBufferLine(uint8_t ix);
void clearBlock(uint8_t x,uint8_t y,uint8_t len);
void reloadSaucerFrames();
void resetGameScreen();
void resetMissiles();
void resetGame();
void resetInvaders();
void doPricingAnim();
void getInvaderMinMax();

void drawInvader(uint8_t ix, uint8_t col, uint8_t row);
void drawInvaders();
int8_t lowestInvaderForCol(int8_t tryColIX);
void getBombPosByPlayerPos(uint8_t ix, int8_t &col, int8_t &invaderRow);
void getBombPosBySequence(uint8_t ix, int8_t &col, int8_t &invaderRow);
void shootMissile(uint8_t ix);
void drawPlayer();
void testSpriteShieldCollisions();
void setFreq(const uint8_t chan, uint8_t freq);
void updateGame();
void setState(GameState nextState);
void gameLoop();


// Implementations

void drawNumber(const uint8_t x, const uint8_t y, uint16_t num, const uint8_t digits){
  uint8_t *p=screenRam+(x+(y*BYTES_PER_BUFFER_LINE))+(digits-1);
  for(uint8_t n=0;n<digits;n++){
    *p--=(fontIX+26+(num%10));
    num/=10;
  }
}

void drawBits(uint8_t x, uint8_t y, uint16_t num){
  uint8_t *p=screenRam+(x+(y*BYTES_PER_BUFFER_LINE))+16;
  for(uint8_t n=0;n<16;n++){
    *p--=(fontIX+26+(num&1));
    num>>=1;
  }
}

void drawText(uint8_t x, uint8_t y, const uint8_t *txt, uint8_t len){
  uint8_t *p=screenRam+(x+(y*BYTES_PER_BUFFER_LINE));
  while(len--){
    *p++=(pgm_read_byte(txt++));
  }
}

uint8_t drawGameText(const uint8_t *txt, uint8_t maxLen=0){
  uint8_t x=pgm_read_byte(txt+0)+2;
  uint8_t y=pgm_read_byte(txt+1);
  uint8_t len=pgm_read_byte(txt+2);
  uint8_t writeLen=(maxLen>0?maxLen:len);
  drawText(x,y,txt+3,writeLen);
  return ((writeLen==len)?1:0);
}

uint8_t drawSlowText(const uint8_t *txt){
  if(++giy==5){
    ++gix;
    giy=0;
    uint8_t r=drawGameText(txt,gix);
    if(r){
      gix=0;
    }
    return r;
  }else{
    return 0;
  }
}

void drawAdvanceText(){
  drawGameText(scoreAdvanceTxt);
  uint8_t *p=screenRam+(10+(18*BYTES_PER_BUFFER_LINE));
  uint8_t ix=225;
  for(uint8_t n=0;n<4;n++){
    *p++=ix++;
    *p++=ix++;
    p+=(BYTES_PER_BUFFER_LINE*2)-2;
  }
}

void hideSprites(){
  for(uint8_t n=0;n<NUM_SPRITES;n++){
    setSpritePos(n,255,n*16);
  }
}

void drawScoreLine(){
  uint8_t y=pgm_read_byte(&scoreNumY);
  for(uint8_t n=0;n<3;n++){
    uint8_t x=pgm_read_byte(scoreNumX+n)+2;
    drawNumber(x,y,(n==0?gameData.score[0]:(n==1?hiScore:gameData.score[1])),4);
  }
}

void drawCredits(){
    drawNumber(26,30,gameData.credit,2);
}

void addScore(uint8_t s){
  if(DEMO_MODE){
    return;
  }
  gameData.score[gameData.currentPlayerIX]+=(s*10);
  if(gameData.score[gameData.currentPlayerIX]>hiScore){
    hiScore=gameData.score[gameData.currentPlayerIX];
    SET_NEW_HI;
  }
  drawScoreLine();
}

void drawLives(){
  const uint8_t l=gameData.lives[gameData.currentPlayerIX];
  drawNumber(2,30,l,1);
  uint8_t *p=screenRam+(4+(30*BYTES_PER_BUFFER_LINE));
  for(uint8_t n=1;n<4;n++){
    if(l>n){
      *p++=60;
      *p++=61;
    }else{
      *p++=blankTile;
      *p++=blankTile;
    }
  }
}

uint8_t wipeScreen(uint8_t numRows){
  uint8_t *p=screenRam+gix;
  while(numRows--){
    *p++=blankTile;
    *p++=blankTile;
    p+=(BYTES_PER_BUFFER_LINE-2);
  }
  drawGameText(scoreLineTxt);
  drawGameText(creditTxt);
  drawCredits();
  drawScoreLine();
  gix+=2;
  if(gix==BYTES_PER_RASTER){
    gix=0;
    return 1;
  }
  return 0;
}

/**
 * @brief Draw alien sprite used on title screens. This is used to bomb the extra C on CCOIN, and also replace the upside-down Y in PLAY
 * 
 * @param x X pixel position
 * @param yC Y character position (sprite is only ever on one of two character lines)
 * @param ix Type of alien - 0=alone, 1=pulling upside down Y to right, 2=pushing Y to left
 */
void drawTitleAlien(uint8_t x,uint8_t yC,uint8_t ix){
  uint8_t baseIX=233;
  uint8_t len=2;
  if(ix==1){
    baseIX=156;
    len=3;
  }else if(ix==2){
    baseIX=132;
    len=3;
  }
  uint8_t xChar=(x>>3);
  uint8_t *p=screenRam+xChar+(yC*BYTES_PER_BUFFER_LINE);
  uint8_t frm=baseIX+((x&0x07)*len);
  for(uint8_t n=0;n<len;n++){
    if(++xChar<33){
      *p++=frm++;
    }
  }
}

// Used at start of level
void initShields(){
  for(uint16_t n=0;n<256;n++){
    shieldLineBuffer[n]=pgm_read_byte(shieldLine+n);
  }
  // Also draw the line under shields while we're at it...
  uint8_t *p=screenRam+(2+(29*BYTES_PER_BUFFER_LINE));
  for(uint8_t n=0;n<BYTES_PER_RASTER-4;n++){
    *p++=224;
  }
}

void clearShields(){
  for(uint16_t n=0;n<256;n++){
    shieldLineBuffer[n]=0;
  }
}

/**
 * @brief Returns information about a bit in the invaderBitmap array.
 * 
 * @note         NO_INVADER   (-1) if bit not set
 * @note         INVADER_HERE  (0) if only this bit
 * @note         INVADER_LEFT  (1) if also bit to left, none to right, 
 * @note         INVADER_RIGHT (2) if bit to right, none to left, 
 * @note         INVADER_BOTH  (3) if bits to both sides of this one

 * @param bitPos The position in the array, from 0-54. This is split across the 5 arrays
 * @return int8_t 
 */
int8_t bitAt(uint8_t bitPos){
  uint8_t ix=(bitPos/INVADERS_PER_ROW);
  uint8_t b=(bitPos%INVADERS_PER_ROW);
  int8_t ret=(gameData.invaderBitmap[ix]&(1<<b))?0:-1;
  if(ret==-1){
    return ret;
  }
  if( (b>0) && (gameData.invaderBitmap[ix]&(1<<(b-1))) ){
    ret+=1;
  }
  if( (b<(INVADERS_PER_ROW-1)) && gameData.invaderBitmap[ix]&(1<<(b+1)) ){
    ret+=2;
  }
  return ret;
}

/**
 * @brief Calculate the minimum and maximum cursor value in the x axis to allow remaining aliens to hit edges of screen before dropping.. Should be called at level start, and after any alien is killed.
 * 
 */
void getInvaderMinMax(){
    gameData.cursorXMax=-128; 
    gameData.cursorXMin=127;
    for(uint8_t i=0;i<11;i++){
        uint16_t bitTest=(1<<i);
        for(uint8_t n=0;n<INVADER_ROWS;n++){
            if(gameData.invaderBitmap[n]&bitTest){
                if(i>gameData.cursorXMax){
                    gameData.cursorXMax=i;
                }
                if(i<gameData.cursorXMin){
                    gameData.cursorXMin=i;
                }
            }
        }
    }
    gameData.cursorXMin<<=1;
    gameData.cursorXMax<<=1;
    gameData.cursorXMin=3-gameData.cursorXMin;
    gameData.cursorXMax=26-gameData.cursorXMax;
}

uint8_t isLastInvaderToUpdate(){
  uint8_t bitPos=gameData.invaderUpdateBit+1;
  for(uint8_t n=bitPos;n<NUM_INVADERS;n++){
    if(bitAt(n)!=NO_INVADER){
      return 0;
    }
  }
  return 1;
}

uint8_t incUpdateBit(){
  uint8_t tries=0;
  while(tries++<NUM_INVADERS){
    if(++gameData.invaderUpdateBit>=NUM_INVADERS){
      gameData.invaderUpdateBit=0;
    }
    if(bitAt(gameData.invaderUpdateBit)!=NO_INVADER){
      return 1;
    }
  }
  return 0;
}

uint8_t revealInvaders(){
  if(gameData.revealedInvaders==0){ // Store the cursor etc for use in game
    initShields();
  }
  if(gameData.revealedInvaders<NUM_INVADERS){
    ++gameData.revealedInvaders;
    gameData.invaderUpdateBit=NUM_INVADERS-1;
    return 0;
  }
  return 1;
}

// Reset the state of the "machine"" at boot. Only occurs once.
void resetMachine(){
    hideSprites();
    setScroll(0,0);
    resetGame();
    gameData.credit=0;
    gameData.demoModeCount=1;
}

void setReloadRate(){
  if(DEMO_MODE){
    gameData.reloadRate=7;
    return;
  }
  gameData.reloadRate=72;
  for(uint8_t n=0;n<5;n++){
    const uint16_t rR=((uint16_t)pgm_read_byte(alienReloadRate+(n*2)))*100;
    if(rR>gameData.score[gameData.currentPlayerIX]){
      gameData.reloadRate=pgm_read_byte(alienReloadRate+1+(n*2));
      break;
    }
  }
  
}

void clearBufferLine(uint8_t ix){
  uint8_t *p=screenRam+(((uint16_t)ix)*BYTES_PER_BUFFER_LINE);
  for(uint8_t n=0;n<BYTES_PER_BUFFER_LINE;n++){ // Draw 10 rows, to also handle dropping invaders...
    *p++=blankTile;
  }
}

void clearBlock(uint8_t x,uint8_t y,uint8_t len){
  uint8_t *p=screenRam+x+(((uint16_t)y)*BYTES_PER_BUFFER_LINE);
  for(uint8_t n=0;n<len;n++){ // Draw 10 rows, to also handle dropping invaders...
    *p++=blankTile;
  }
}

void reloadSaucerFrames(){
  if(gameData.remainingInvaders<9){
    gameData.saucerTmr=9999;
  }
  gameData.saucerTmr=pgm_read_byte(&framesToSaucer)+(gameData.remainingInvaders*4);
}

void resetGameScreen(){
  drawScoreLine();
  hideSprites();
  processSprites();
  clearShields();
}

void resetMissiles(){
  hideSprites();
  for(uint8_t n=0;n<NUM_MISSILES;n++){
    Missile *m=gameData.missile+n;
    m->dirY=0;
    m->expTmr=0;
    m->colIX=0;
    m->baseDefIX=(n==0?14:(n-1)*4);
    m->expDefIX=(n==0?15:12);
    if(n==1 || n==2){
      m->colIX=pgm_read_byte(bombColRange+((n-1)*2));
    }
  }
}

void resetGame(){
  gameData.currentPlayerIX=0;
  for(uint8_t n=0;n<2;n++){
    gameData.level[n]=0;
    gameData.lives[n]=3;
    gameData.score[n]=0;
  }
  resetInvaders();
}

/**
 * @brief Used to reset vars at the start of a level
 * 
 */
void resetInvaders(){
  setReloadRate();
  for(uint8_t n=0;n<INVADER_ROWS;n++){
    gameData.invaderBitmap[n]=(0x07<<8)+0xff;
  }

  gameData.cursorX=pgm_read_byte(&invadersStartX)+2;
  gameData.cursorY=pgm_read_byte(invadersStartY+gameData.level[gameData.currentPlayerIX]);
  gameData.invaderDir=2;
  gameData.invaderOffset=0;
  gameData.revealedInvaders=0;
  gameData.invaderUpdateBit=0;
  gameData.invaderPauseTmr=0;
  gameData.gameFlags&=((1<<TWO_PLAYER_GAME_BIT)||(1<<NEW_HI_BIT));
  gameData.gameTmr=0;
  gameData.demoModeTmr=0;
  gameData.alienMissileSpeed=4;
  gameData.movTmr=gameData.reloadRate;
  gameData.remainingInvaders=NUM_INVADERS;
  gameData.saucerTmr=((35*50)/3);  // Initially about 35 seconds, then resets from framesToSaucer+(remainingInvaders*4)
  gameData.beatIX=0;
  gameData.beatTmr=0;
  gameData.beatDelayIX=0;

  resetMissiles();
  getInvaderMinMax();
}

void doTitleAnim(){
    switch(giu){
      case 0:
        if(wipeScreen()){
          giu=1;
        }
      break;
      case 1:
        if(drawSlowText((gameData.demoModeCount&1)?plaATxt:playTxt)){
          giu=2;
        }
      break;
      case 2:
        if(drawSlowText(spaceInvadersTxt)){
          giu=3;
        }
      break;
      case 3:
        if(++gix==60){
          drawAdvanceText();
          gix=0;
          giu=4;
        }
      break;
      case 4:
        if(drawSlowText(pointsMysteryTxt)){
          giu=5;
        }
      break;
      case 5:
        if(drawSlowText(points30Txt)){
          giu=6;
        }
      break;
      case 6:
        if(drawSlowText(points20Txt)){
          giu=7;
        }
      break;
      case 7:
        if(drawSlowText(points10Txt)){
          giu=8;
        }
      break;
      case 8:
        if(++gix==100){
          if(gameData.demoModeCount&1){
            giu=9;
            gix=31*8;
            giy=0;
          }else{
            setState(demo);
          }
        }
      break;
      case 9:
        if(giy==0){
          if(gix>18*8){
            --gix;
            drawTitleAlien(gix,8,0);
          }else{
            giy=1;
            gix=17*8;
          }
        }else if(giy==1){
          ++gix;
          clearBlock(17,8,15);
          drawTitleAlien(gix,8,1);
          if(gix==248){
            giy=2;
            gix=0;
          }
        }else if(giy==2){
          if(++gix==75){
            giy=3;
            gix=248;
          }
        }else if(giy==3){
          if(--gix>(17*8)){
            clearBlock(17,8,15);
            drawTitleAlien(gix,8,2);
          }else if(gix==(17*8)){
            drawGameText(playTxt);
            drawTitleAlien(gix+8,8,0);
          }else if(gix==100){
            clearBlock(18,8,2);
          }else if(gix==0){
            setState(demo);
          }
        }
      break;
    }
}

void doPricingAnim(){
    switch(giu){
     case 0:
        if(wipeScreen()){
          drawGameText((gameData.demoModeCount&1)?insertCCoinTxt:insertCoinTxt);
          giu=1;
        }
      break;
      case 1:
        if(drawSlowText(oneOrTwoPlayersTxt)){
          giu=2;
        }
      break;
      case 2:
        if(drawSlowText(onePlayerOneCoinTxt)){
          giu=3;
        }
      break;
      case 3:
        if(drawSlowText(twoPlayersTwoCoinsTxt)){
          giu=4;
        }
      break;
      case 4:
        if(++gix==100){
          ++gameData.demoModeCount;
          if((gameData.demoModeCount&1)==0){
            giu=5;
            gix=0;
          }else{
            setState(title);
          }
        }
      break;
      case 5:
        if(++gix<((17*8)+1)){
          clearBlock(0,5,20);
          drawTitleAlien(gix,5,0);
        }else if(gix==((17*8)+16)){
          giu=6;
          gix=6*8;
        }
      break;
      case 6:
        if(++gix<(14*8)-4){
          setSpriteDef(0,gix&0x03);
          setSpritePos(0,(17*8)+3,gix);
        }else if(gix==(14*8)-4){
          setSpriteDef(0,12);
          setSpritePos(0,(17*8),gix);
        }else if(gix==(14*8)+10){
          clearBlock(17,14,1);
          hideSprites();
        }else if(gix==222){
          setState(title);
        }
        processSprites();
      break;
    }
}

void runDemo(){
  switch(giu){
    case 0:
      if(wipeScreen()){
        giu=1;
        gameData.invaderUpdateBit=0;
      }
    break;
    case 1:
      if(revealInvaders()){
        giu=2;
      }
    break;
    case 2:
      if(PLAYER_DEAD || (gameData.gameTmr==2500)){
        setState(pricing);
      }
    break;
  }
  if(giu>0){
    updateGame();
  }
}

void doWait(uint8_t frames){
  while(frames-->0){
    waitFrame();
  }
}

#endif