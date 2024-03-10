extern "C" { void storeHi(); }

uint8_t getWillyFrame(){
  uint8_t willyFrame=((spriteData[8].x>>1)+1)&0x03;
  if(willyFlags[1]&(1<<4)){
    willyFrame+=4;
  }
  return willyFrame;
}

void setWillyPos(uint8_t willyX,uint8_t willyY){
  uint8_t willyFrame=getWillyFrame();
  setSpriteDef(8,pgm_read_byte(gameHeader.willyFrameIX+willyFrame));
  setSpritePos(8,willyX,willyY);
}

void setWillyInitialPos(){
  uint8_t willyFrame=(willyFlags[1]>>1)&0x07;
  if(willyFlags[1]&(1<<4)){
    willyFrame=(willyFrame+4)&0x07;
  }
  uint8_t willyX=((willyFlags[0]<<3)-2)+((willyFrame&0x03)<<1);
  uint8_t willyY=( ( (willyFlags[0]>>5) + ((willyFlags[1]&0x01)<<3) ) << 3 ) + ( (29*8)-vScroll );
  setWillyPos(willyX,willyY);
  willyFlags[0]=0;  // Used for velocity after level has loaded and Willy is in initial position
}

boolean loadLevel(){
  const uint8_t writeRow=21;
  
  // If already showing a level, scroll it down a bit before starting to write this new one!
  if(levelState==levelReady || levelState==levelLoaded){
    hideScore();
    levelState=levelLoaded;
    vScroll-=vScrollSub;
    vMoveSprites(vScrollSub,true);
    setSpritePos(8,255,16*8);
    if((vScroll&0x07)==0){
      uint8_t *p=screenRam+(BYTES_PER_BUFFER_LINE * (4+(((vScroll>>3)-2)%28)) );
      for(uint8_t n=0;n<32;n++){
        *p++=190;
      }
      if(vScrollSub<4){
        vScrollSub<<=1;
      }
      if((vScroll>>3)==2){
        drawHLine(screenRam+(31*BYTES_PER_BUFFER_LINE),32,190);
        levelState=levelNotLoaded;
      }
    }
    return false;
  }
  
  if(levelState==levelNotLoaded){
    levelState=levelLoading;
    levelLoadCnt=0;
    vScrollSub=4;
    vScroll=(PIXELS_PER_CHARACTER*(19+28));
    unpackBytes( screenRam+(BYTES_PER_BUFFER_LINE*writeRow), NULL, gameLevel[level].levelName,BYTES_PER_BUFFER_LINE, 5 );
    uint8_t fontIX=pgm_read_byte(&gameHeader.fontIX);
    for(uint8_t n=0;n<32;n++){
      screenRam[(BYTES_PER_BUFFER_LINE*writeRow)+n]+=(fontIX+12);  // Original game had ABC at start of font, we have 0123456789,. so we must add 12 chars to get original text!
    }

    for(uint8_t n=0;n<5;n++){
      pickup[n]=0;
      uint8_t x=pgm_read_byte(gameLevel[level].pickupPos+(n*2));
      if(x<31){
        pickup[n]=((pgm_read_byte(gameLevel[level].pickupPos+((n*2)+1))+4)*BYTES_PER_BUFFER_LINE)+x;
      } 
    }
    
    return false;
  }

  if(levelState==levelLoading){
    vScroll-=vScrollSub;
    vMoveSprites(vScrollSub,false);
    if((vScroll&0x07)==0){
      if(vScrollSub<4){
        vScrollSub<<=1;
      }
      ++levelLoadCnt;
      if(levelLoadCnt>1){

        uint8_t mapRow=(writeRow-(levelLoadCnt));
        unpackBytes( 
          screenRam+(BYTES_PER_BUFFER_LINE*mapRow), 
          gameLevel[level].tileIX,gameLevel[level].map+(12*(17-levelLoadCnt)),
          32,3
        );

        if(levelLoadCnt==17){
            if(level!=19){
              drawHLine(screenRam+(31*BYTES_PER_BUFFER_LINE),32,255);
            }
            const uint8_t *p=gameLevel[level].guardianFrameIX;
            const Guardian *g=gameLevel[level].guardian;
            const uint8_t *w=gameLevel[level].willyInit;
            willyFlags[0]=pgm_read_byte(&w[0]);
            willyFlags[1]=pgm_read_byte(&w[1]); // bit 8 - 1=has vertical guardians
            levelFlags=pgm_read_byte(&gameLevel[level].flags);
            for(uint8_t n=0;n<8;n++){
              spriteFrameIX[n]=pgm_read_byte(p++);
            }
            verticalGuardian=0;
            eugeneGuardian=0;
            kongGuardian=0;
            boolean hasVertical=(willyFlags[1]&(1<<5));
            hideSprites();
            
            for(uint8_t n=0;n<8;n++){
              const uint8_t flags=pgm_read_byte(&g[n].flags);
              if(flags!=0xff){
                spriteMax[n]=pgm_read_byte(&g[n].maxPos);
                spriteMin[n]=pgm_read_byte(&g[n].minPos);
  
                uint8_t sFrame=flags>>5;
                uint8_t mT=(flags>>3)&0x03;
                uint8_t mSpeed=(flags&0x07);
                uint8_t spX=pgm_read_byte(&g[n].startPos[0]);
                uint8_t spY=pgm_read_byte(&g[n].startPos[1])-8;
                if( (mT==0) && hasVertical && sFrame<4){
                  sFrame+=4;
                }
                if(mT!=0){
                  verticalGuardian|=(1<<n);
                  spriteMax[n]+=38; // Adjust for scrolled position of level on display
                  spriteMin[n]+=38;
                }else{
                  spriteMin[n]-=2;
                  spriteMax[n]+=4;
                  spX+=(sFrame<<1);
                  mSpeed++;
                }
                if(!hasVertical){
                  spriteDir[n]=( (sFrame<4) ? mSpeed:-mSpeed );
                }else{
                  spriteDir[n]=( ( (mT==0 && sFrame<2) || (mT>0 && sFrame<6) ) ? mSpeed:-mSpeed );
                }
  
                if(mT==3){
                  eugeneGuardian|=(1<<n);
                  spriteDir[n]=1;
                  setSpriteDef(n,pgm_read_byte(&gameHeader.eugeneIX));
                }else{
                  if(mT==2){
                    kongGuardian|=(1<<n);
                  }
                  setSpriteDef(n,spriteFrameIX[sFrame]);
                }
                spriteFrame[n]=sFrame;
                setSpritePos(n,spX,spY);
              }
            }
  
          vScrollSub=1;
          levelState=levelPositioning;
          giy=0;
        } // if(levelLoadCnt==16)
      }
    }

  // Scroll level down into position, moving Sprites along with it, then position Willy at his start position ready to play
  }else if(levelState==levelPositioning){
    if(vScroll>(25*8)){
      vScroll-=4;
      vMoveSprites(4,false);
    }else{
      levelState=levelPositioning2;
      giy=4;
      giz=0;
      SET_TILE_ANIMATION_ON;
    }
    
  }else if(levelState==levelPositioning2){
    vScroll+=giy;
    vMoveSprites(-giy,false);
    if(++giz==3){
      giz=0;
      giy>>=1;
      if(giy==0){
        levelState=levelPositioning3;
        giy=1;
        giz=0;
      }
    }
    
  }else if(levelState==levelPositioning3){
    vScroll-=giy;
    vMoveSprites(giy,false);
    if(++giz==4){
      giz=0;
      giy<<=1;
      if(giy==8){
        levelState=levelReady;
        setWillyInitialPos();
        // For final cavern we need to activate the font switching in the interrupt
        // to allow for extra tile graphics at the top of the gameplay area...
        if(level==19){
          for(uint16_t n=0;n<256;n++){
            uint8_t b=pgm_read_byte(gameHeader.cavernLayout+n);
            screenRam[(4*BYTES_PER_BUFFER_LINE)+n]=b;
          }
          altFontLine=33;
          restoreFontLine=104;
          altFontIX=1;
        }
        return true;
      }
    }
  }

  return false;
}

void setState(GameState nextState){
  
  switch(nextState){

    case title:
    readInput();
    setAudioType(titleMusic);
    drawOutline(0,1,1);
    break;
    case levelStart:
    gameFlags=0;
    setAudioType(gameMusic);
    drawAirBarUI();
    restoreFontLine=0;
    altFontLine=255;
    break;
    case playing:
    //jmpV=0;
    //sfxV=0;
    break;
    case levelClear:
    restoreFontLine=0;
    altFontLine=255;
    setAudioType(noMusic);
    break;
    case lostLife:
    setAudioType(noMusic);
    air=0;
    hideSprites();
    levelState=levelNotLoaded;
    break;
    case gameOver:
    restoreFontLine=0;
    altFontLine=255;
    setAudioType(noMusic);
    clearScreen(0,32,190);
    vScroll=(PIXELS_PER_CHARACTER*(19+28));
    setSpriteDef(0,pgm_read_byte(gameHeader.willyFrameIX+0));
    setSpritePos(0,122,104);
    setSpriteDef(1,pgm_read_byte(&gameHeader.plinthIX));
    setSpritePos(1,120,120);
    setSpriteDef(2,pgm_read_byte(&gameHeader.bootIX));
    setSpritePos(2,255,0);
    levelLoadCnt=0;
    break;
    case gameWon:
    setAudioType(noMusic);
    break;
    default:
    break;
  }
  giu=giv=giw=gix=giy=giz=0;
  gameState=nextState;
}

void doScroller(){
  if((giw%4)==0){
    uint8_t ix=(giw/4)*3;
    unpackBytes(scrollBuff, NULL, gameHeader.demoModeText+ix, 4, 6);
    uint8_t fontIX=pgm_read_byte(&gameHeader.fontIX);
    for(uint8_t n=0;n<4;n++){
      scrollBuff[n]+=fontIX;
    }
    if(ix+3>=pgm_read_byte(&gameHeader.demoModeTextLength)){
      giw=0;
    }else{
      ++giw;
    }
  }else{
    ++giw;
  }
  uint8_t *p=screenRam+((1*32)+1);
  for(uint8_t n=0;n<30;n++){
    *p=*(p+1);
    ++p;
  }
  --p;
  *p=scrollBuff[0];
  for(uint8_t n=0;n<3;n++){
    scrollBuff[n]=scrollBuff[n+1];
  }
}

void drawBigText(uint8_t x,uint8_t y,uint8_t len,const uint8_t *src){
  uint8_t xPos=x;
  while(len--){
    uint8_t c=pgm_read_byte(src++);
    if(c==0x25){
      x=xPos;
      y+=2;
    }else{
      uint8_t chr=pgm_read_byte(&gameHeader.titleLayout[c]);
      screenRam[(y*BYTES_PER_BUFFER_LINE)+x]=chr;
      chr=pgm_read_byte(&gameHeader.titleLayout[c+48]);
      screenRam[((y+1)*BYTES_PER_BUFFER_LINE)+x]=chr;
      ++x;
    }
  }
}

void drawMenu(uint8_t *txt,uint8_t len, uint8_t xPos, uint8_t yPos){
  drawBigText(xPos,yPos,len,gameHeader.optionText);
  uint8_t c,chr;
  for(uint8_t n=0;n<5;n++){
    c=(cursorY==n?40:0);
    chr=pgm_read_byte(&gameHeader.titleLayout[c]);
    screenRam[((yPos+(n*2))*BYTES_PER_BUFFER_LINE)+12]=chr;
    chr=pgm_read_byte(&gameHeader.titleLayout[c+48]);
    screenRam[((yPos+1+(n*2))*BYTES_PER_BUFFER_LINE)+12]=chr;
  }

  // Draw the start level
  uint8_t sL=startLevel+1;
  for(uint8_t n=0;n<2;n++){
    c=(sL%10)+1;
    chr=pgm_read_byte(&gameHeader.titleLayout[c]);
    screenRam[((yPos+4)*BYTES_PER_BUFFER_LINE)+xPos+14-n]=chr;
    chr=pgm_read_byte(&gameHeader.titleLayout[c+48]);
    screenRam[((yPos+5)*BYTES_PER_BUFFER_LINE)+xPos+14-n]=chr;
    sL/=10;
  }

  if((currentFrame%4)>1){
    for(uint8_t n=0;n<17;n++){
      screenRam[((yPos+(cursorY*2))*BYTES_PER_BUFFER_LINE)+n+xPos]=0;
      screenRam[((yPos+1+(cursorY*2))*BYTES_PER_BUFFER_LINE)+n+xPos]=0;
    }
  }

  if(!PRESSING_UP && !PRESSING_DOWN && !PRESSING_LEFT && !PRESSING_RIGHT && !PRESSING_JUMP){
    gameFlags&=~GF_JUMP_LOCKOUT;
  }else{
    giu=0;
    giv=0;
  }
  
  if((gameFlags&GF_JUMP_LOCKOUT)==0){
    if(PRESSING_UP && cursorY>0){
      gameFlags|=GF_JUMP_LOCKOUT;
      --cursorY;
    }
    if(PRESSING_DOWN && cursorY<3){
      gameFlags|=GF_JUMP_LOCKOUT;
      ++cursorY;
    }
  }
}

boolean drawTitleGfx(){
  altFontLine=0;
  restoreFontLine=212;
  altFontIX=2;
  uint8_t *p=screenRam;
  if(giz==0){
    clearScreen(4,28,0);
  }
  // 11 rows down = start of willy
  uint16_t ix=11*16;
  if(giz<17){
    for(uint8_t y=32-giz;y<32;y++){
      for(uint8_t x=0;x<16;x++){
        uint8_t chr=pgm_read_byte(&gameHeader.titleLayout[ix++]);
        screenRam[(y*BYTES_PER_BUFFER_LINE)+x]=chr;
      }
    }
    if(++giz==17){
      return true;
    }
  }
  
  ix=6*16;
  for(uint8_t y=4;y<9;y++){
    for(uint8_t x=0;x<16;x++){
      uint8_t chr=pgm_read_byte(&gameHeader.titleLayout[ix++]);
      screenRam[(y*BYTES_PER_BUFFER_LINE)+(x+20)]=chr;
    }
  }

  return false;
}

uint8_t tileTypeAt(uint16_t ix){

  if( (level==19) && (ix<(12*BYTES_PER_BUFFER_LINE)) ){
    if(screenRam[ix]==0x03){
      return TILE_TYPE_BACKGROUND;
    }
    if(screenRam[ix]==0x3F){
      return TILE_TYPE_PICKUP;
    }
    if(screenRam[ix]==0x36 || screenRam[ix]==0x01){
      return TILE_TYPE_WALL;
    }
    if(screenRam[ix]>=55 && screenRam[ix]<55+8){
      return TILE_TYPE_PORTAL;
    }
  }

  for(uint8_t n=0;n<9;n++){
    uint8_t tIX=pgm_read_byte(gameLevel[level].tileIX+n);
    if( (n==2 || n==4) && (tIX==(screenRam[ix]&0xf8)) ){
      return n;
    }else if(tIX==screenRam[ix]){
      return n;
    }
  }
  
  uint8_t portalIX=pgm_read_byte(&gameHeader.portalIX);
  if(screenRam[ix]>=portalIX && screenRam[ix]<portalIX+8){
    return TILE_TYPE_PORTAL;
  }
  
  return 0;
}

// Sets a 3bit field:
// bit 0 = can go right, bit 1 = can go left, bit 2 = in air, bit 3 = cannot jump
uint8_t getAvailableDirectionFlags(){
  uint8_t sY=spriteData[8].y;
  uint8_t sX=spriteData[8].x;
  uint8_t r=0xff;

  uint16_t bufferRow=(BYTES_PER_BUFFER_LINE*(sY>>3));
  
  // Check if player is blocked from moving left by wall
  uint16_t checkLeft=bufferRow+(sX>>3);
  if(tileTypeAt(checkLeft)==3 || tileTypeAt(checkLeft+BYTES_PER_BUFFER_LINE)==3){
    r&=~CAN_MOVE_LEFT;
  }

  // Check if player is blocked from moving right by wall
  uint16_t checkRight=bufferRow+((sX+12)>>3);
  if(tileTypeAt(checkRight)==3 || tileTypeAt(checkRight+BYTES_PER_BUFFER_LINE)==3){
    r&=~CAN_MOVE_RIGHT;
  }

  // Check to see if player blocked from jumping by wall tiles above - also causes velocity to change to zero if above zero!
  uint8_t wd=(willyFlags[1]&WF_FACING_LEFT);
  uint8_t fr=sX&0x03;
  int8_t willyDir2=( (wd && fr!=0) || (!wd && fr==2) ? 2:0); // Block checking depends on direction and frame to match original game.
  
  checkLeft=bufferRow-BYTES_PER_BUFFER_LINE+(sX+(willyDir2)>>3)+1;
  checkRight=bufferRow-BYTES_PER_BUFFER_LINE+((sX+(willyDir2)+10)>>3)-1;
  
  if(tileTypeAt(checkLeft)==3 || tileTypeAt(checkRight)==3){
    r&=~CLEAR_ABOVE;
  }

  // Sprites are always 1 pixel further down than their set position, so we need to add 1 to align with 8pixel tile height
  if(((sY+1)&0x07) || (yVel!=0)){ // Can't jump or change direction if not on floor
    r&=~NOT_IN_AIR; // Always in air if not aligned with tile boundaries!
  }

  checkLeft=bufferRow+(BYTES_PER_BUFFER_LINE*2)+((sX+willyDir2)>>3)+1;
  checkRight=bufferRow+(BYTES_PER_BUFFER_LINE*2)+((sX+(willyDir2)+10)>>3)-1;
  
  // Assume nothing under player, then set bit if non-background block found to left or right under player
  // Note that player can be both in the air, AND with a block under them.
  r&=~BLOCK_UNDER;
  r&=~ON_CONVEYOR;
  uint8_t onGround=((r&NOT_IN_AIR) && (yVel==0))?1:0;
  for(uint8_t n=0;n<2;n++){
    uint16_t ix=(n==0?checkLeft:checkRight);
    uint8_t tt=tileTypeAt(ix);

    switch(tt){
      case TILE_TYPE_CRUMBLING:  // Crumbling tile
        r|=BLOCK_UNDER;
        if(onGround){
          ++screenRam[ix];
          if((screenRam[ix]&0x07)==0x07){
            screenRam[ix]=pgm_read_byte(gameLevel[level].tileIX+TILE_TYPE_BACKGROUND);  // Background
          }
        }
      break;
      
      case TILE_TYPE_FLOOR:  // Floor, wall and spare tiles do nothing when player is above them (spare can be switches in Kong rooms if player hits one with head)
      case TILE_TYPE_WALL:
      case TILE_TYPE_SPARE:
        r|=BLOCK_UNDER;
      break;
      
      case TILE_TYPE_CONVEYOR:
        r|=BLOCK_UNDER;
        r|=ON_CONVEYOR;
        if(onGround){
          if(levelFlags&(1<<5)){  // Conveyor goes right
            if(xVel==0 || xVel>0){// If not moving or moving right, then cannot move left
              r&=~CAN_MOVE_LEFT;
              if(wd==0){  // Willy facing right
                xVel=2;
              }
            }
          }else{
            if(xVel==0 || xVel<0){// If not moving or moving left, then cannot move right
              r&=~CAN_MOVE_RIGHT;
              if(wd==1){  // Willy facing left
                xVel=-2;
              }
            }
          }
        }
      break;
      default:
      break;      
    }
  }

  if((r&BLOCK_UNDER)==0){
    r&=~NOT_IN_AIR; // No block of any type player can land on, so player is "in the air"
  }

  return r;

}

CollisionType checkCollisions(){
  CollisionType r=hitNone;
  uint8_t willyFrame=getWillyFrame();
  uint8_t sX=spriteData[8].x;
  uint8_t sY=spriteData[8].y-4; // Sprite is always 1 above tile alignment due to scroll position
  uint16_t bufferRow=(BYTES_PER_BUFFER_LINE*(sY>>3));

  uint8_t wd=(willyFlags[1]&WF_FACING_LEFT);
  uint8_t fr=sX&0x03;
  int8_t willyDir2=( (wd && fr!=0) || (!wd && fr==2) ? 2:0); // Block checking depends on direction and frame to match original game.
  
  uint16_t checkLeft=bufferRow+((sX+willyDir2)>>3);
  uint16_t checkRight=bufferRow+((sX+(willyDir2)+10)>>3);

  uint8_t ascii=pgm_read_byte(&gameHeader.fontIX);
  uint8_t hp=0;
  
  for(uint8_t n=0;n<2;n++){
    for(uint8_t i=0;i<2;i++){
      uint16_t checkSide=(i==0?checkLeft:checkRight);
      uint8_t tt=tileTypeAt(checkSide);
      if(tt==TILE_TYPE_PICKUP){
        if( (level==19) && (checkSide<(12*BYTES_PER_BUFFER_LINE)) ){
          screenRam[checkSide]=255;
        }else{
          screenRam[checkSide]=ascii+((29*8)-vScroll);
        }

        return hitPickup;
      }
      if(tt==TILE_TYPE_NASTY1 || tt==TILE_TYPE_NASTY2){
        return hitNasty;
      }
      if(tt==TILE_TYPE_PORTAL){
        hp+=(i+1);
      }
      if((tt==TILE_TYPE_SPARE) && (willyFlags[1]&WF_HAS_KONG)){
        return hitSwitch;
      }
    }
    
    if(hp>2){
      return hitPortal;
    }
    
    checkLeft+=BYTES_PER_BUFFER_LINE;
    checkRight+=BYTES_PER_BUFFER_LINE;
  }
    
  return r;
}

void gameLoop(){
  if(currentFrame!=lastFrame){
    lastFrame=currentFrame;
    //vScroll+=random(1);
    readInput();
    playAudio();
    switch(gameState){
      
      case idle:
      break;
      
      case title:
      
      if(gix==0){
        vScroll=8; //(PIXELS_PER_CHARACTER*(19+28));
        giu=0;
        giv=0;
        gix=1;
        giy=0;
        giz=0;
        level=0;
        cursorY=0;
        levelState=levelNotLoaded;
        hideSprites();
        SET_TILE_ANIMATION_OFF;
      }else if(gix==1){
        if(giy==0){
          if(drawTitleGfx()){
            giy=1;
            giz=0;
            cursorY=0;
          }
        }else if(giy==1){
          drawMenu(gameHeader.optionText,40,14,16);
          if((PRESSING_JUMP || PRESSING_LEFT || PRESSING_RIGHT) && ((gameFlags&GF_JUMP_LOCKOUT)==0)){
            switch(cursorY){
              case 0:
                cursorY=255;
                break;
              case 1:
                if(PRESSING_JUMP){
                  if(IS_MUSIC_ON){
                    SET_MUSIC_OFF;
                  }else{
                    SET_MUSIC_ON;
                  }
                }
                break;
              case 2:
                if(PRESSING_LEFT && startLevel>0){
                  --startLevel;
                }else if(PRESSING_RIGHT && startLevel<19){
                  ++startLevel;
                }
                break;
              case 3:
                if(PRESSING_JUMP){
                  hiScore=0;
                  storeHi();
                }
            }
            gameFlags|=GF_JUMP_LOCKOUT;
          }else if(++giu==50){
            giu=0;
            if(++giv==5){
              gix=2;
              giv=0;
              giy=0;
              clearScreen(3,29,190);
              restoreFontLine=0;
              altFontLine=255;
              hideSprites();
              levelState=levelNotLoaded;
            }
          }
        }
      }else if(gix==2){
        if(loadLevel()){
          gix=3;
          giy=0;
          SET_TILE_ANIMATION_ON;
        }else if(keyState.changed){
          hideSprites();
          giy=0;
          gix=4;
        }
      }else if(gix==3){
        
        if(++giy==128){
          gix=2;
          giy=0;
          giz=0;
          if(level==19){
            levelState=levelNotLoaded;
            hideSprites();
            gix=4;
          }else{
            levelState=levelLoaded;
            if(++level==20){
              level=0;
              gix=0;
            }
          }
        }else if(keyState.changed){
          hideSprites();
          giy=0;
          gix=4;
       }
      }else if(gix==4){
        SET_TILE_ANIMATION_OFF;
        if(giy++<16){
          collapseLevel();
        }else{
          clearScreen(4,28,190);
          restoreFontLine=0;
          altFontLine=255;
          level=0;
          gix=0;
        }
      }
      
      if((currentFrame%6)==0){
        doScroller();
      }
      
      if(cursorY==255){
        level=startLevel;
        score=0;
        lives=START_LIVES;
        clearScreen(3,29,190);
        restoreFontLine=0;
        altFontLine=255;
        hideSprites();
        levelState=levelNotLoaded;
        setState(levelStart);
      }
      
      break;
      
      case levelStart:
        if(loadLevel()){
          xVel=0;
          yVel=0;
          air=2;
          fallDist=0;
          gameFlags=GF_SHOWING_AIR;
          drawOutline(27,2,1);
          showScore();
          setState(playing);
        }
      break;
      
      case playing:
        if((currentFrame%4)==0){
          gameFlags2&=~0x03;
          uint8_t sY=spriteData[8].y-((29*8)-vScroll);
          uint8_t aD=getAvailableDirectionFlags();

          // if moving right, and cannot move right, stop
          if((xVel>0) && (aD&CAN_MOVE_RIGHT)==0){
            xVel=0;
          }
          
          // if moving left, and cannot move left, stop
          if((xVel<0) && (aD&CAN_MOVE_LEFT)==0){
            xVel=0;
          }

          if((aD&CAN_MOVE_LEFT) && (vXVel<0) && (xVel==0)){
            xVel=vXVel;
            if(!(aD&NOT_IN_AIR)){
              vXVel=0;
            }
          }

          if((aD&CAN_MOVE_RIGHT) && (vXVel>0) && (xVel==0)){
            xVel=vXVel;
            if(!(aD&NOT_IN_AIR)){
              vXVel=0;
            }
          }

          // If not in the air...
          if(aD&NOT_IN_AIR){
            vXVel=0;
            xVel=0;
            
            // If it's possible to go right, then allow player to move right
            if(aD&CAN_MOVE_RIGHT){
              if(PRESSING_RIGHT){
                if(WILLY_FACING_LEFT){
                  SET_CHANGE_DIR;
                }else{
                  xVel=2;
                }
              }
            }

            // If it's possible to go left, then allow player to move left
            if(aD&CAN_MOVE_LEFT){
              if(PRESSING_LEFT){
                if(WILLY_FACING_RIGHT){
                  SET_CHANGE_DIR;
                }else{
                  xVel=-2;
                }
              }
            }

            if(aD&ON_CONVEYOR){
              if(CONVEYOR_GOING_LEFT){
                if(WILLY_FACING_RIGHT && xVel==0){
                  SET_CHANGE_DIR;
                }else if( (aD&CAN_MOVE_LEFT) && WILLY_FACING_LEFT ){ // If facing left and conveyor left, move left if possible
                  xVel=-2;
                }
              }else{
                if(WILLY_FACING_LEFT && xVel==0){
                  SET_CHANGE_DIR;
                }else if( (aD&CAN_MOVE_RIGHT) && WILLY_FACING_RIGHT ){ // If facing right and conveyor left, move left if possible
                  xVel=2;
                }
              }
            }

            if(DID_CHANGE_DIR){
              if(WILLY_FACING_RIGHT){
                WILLY_SET_FACE_LEFT;
              }else{
                WILLY_SET_FACE_RIGHT;
              }
            }

            if( ((gameFlags&GF_JUMP_LOCKOUT)==0) && PRESSING_JUMP && (aD&CLEAR_ABOVE) ){
              gameFlags|=GF_JUMP_LOCKOUT;
              sY-=1;
              yVel=-5;
              jmpV=128;
              willyFlags[0]=1;
              vXVel=0;
              if(!DID_CHANGE_DIR){
                // If also pressing left/right, then keep the virtual xVel so we can move in that direction when possible during jump - I know it's weird, but that's how it works!
                if(PRESSING_LEFT && WILLY_FACING_LEFT){
                  vXVel=-2;
                }
                if(PRESSING_RIGHT && WILLY_FACING_RIGHT ){
                  vXVel=2;
                }
              }
            }


          } // If not in air...

          // If not jumping and no block under player, fall.
          if(WILLY_NOT_JUMPING && (aD&BLOCK_UNDER)==0){
            xVel=0;
            yVel=4;
            jmpV=24;
            fallDist=0;
            willyFlags[0]=1;
          }

          // Jumping...
          if(willyFlags[0]==1){
            willyFlags[0]=2;
            yVel++;
            if(yVel>4){ // Continue falling, but no longer move in x-axis
              xVel=0;
              yVel=4;
            }
          }else if(willyFlags[0]==2){
            willyFlags[0]=1;
          }

          // If we are falling and there is a solid tile under us, ensure we don't fall through it!
          if((aD&BLOCK_UNDER) && (yVel>0)){
            uint8_t b=(sY&0x07);
            if((b+yVel)>7){
              sY=(sY&0xf8)+8;
              willyFlags[0]=0;
              yVel=0;
              if(vXVel!=0){
                if((vXVel>0 && CAN_MOVE_RIGHT) || (vXVel<0 && CAN_MOVE_LEFT)){
                  //xVel=vXVel; // Pushes Willy into wall...
                }
                vXVel=0;
              }
            }
          }
          
          // If heading up, and block above is a wall, don't allow us to jump through it
          if(yVel<0 && (aD&CLEAR_ABOVE)==0 && (((sY+1)&0x07)<-yVel)){
            sY&=0xf8;
            sY+=8;
            yVel=0;
            xVel=0;
            vXVel=0;
            fallDist=0;
            willyFlags[0]=0;
          }else{
            if(yVel>0){
              fallDist+=yVel;
            }
            sY+=yVel;
          }

          spriteData[8].x+=xVel;
          setWillyPos(spriteData[8].x,sY+((29*8)-vScroll));

          drawLives();
          
          CollisionType collisionType=checkCollisions();
          
          switch(collisionType){
#ifndef DISABLE_NASTY_COLLISION
            case hitNasty:
              setState(lostLife);
            break;
#endif
            case hitPickup:
              gameFlags|=GF_PICKUP_SND;
              sfxV=200;
              score+=100;
              showScore();
            break;
            case hitPortal:
              if(gameFlags&GF_PORTAL_READY){
                gameFlags|=GF_LEVEL_COMPLETE;
              }
            break;
            case hitSwitch:
            {
              uint8_t tIX=pgm_read_byte(gameLevel[level].tileIX);
              if((spriteData[8].x<128) && ((gameFlags&GF_KONG_SWITCH_1)==0)){
                gameFlags|=GF_KONG_SWITCH_1;
                screenRam[17+(BYTES_PER_BUFFER_LINE*15)]=tIX;
                screenRam[17+(BYTES_PER_BUFFER_LINE*16)]=tIX;
                screenRam[6+(BYTES_PER_BUFFER_LINE*4)]=253;
                spriteMax[5]=144;
              }else if((spriteData[8].x>128) && ((gameFlags&GF_KONG_SWITCH_2)==0)){
                gameFlags|=GF_KONG_SWITCH_2;
                screenRam[15+(BYTES_PER_BUFFER_LINE*6)]=tIX;
                screenRam[16+(BYTES_PER_BUFFER_LINE*6)]=tIX;
                screenRam[18+(BYTES_PER_BUFFER_LINE*4)]=253;
              }
            }
            break;
            default:
            break;
          }
#ifndef DISABLE_SPRITE_COLLISION
          if(collisionBits){
            setState(lostLife);
          }
#endif
          if(keyState.current&(1<<BUTTON_B)){
            gameFlags&=~GF_JUMP_LOCKOUT;
          }
          
        } // (currentFrame%4)==0
        
        if(yVel!=0){
          if(yVel>0 && jmpV<250){
            jmpV+=3;
          }else if(yVel<0 && jmpV>5){
            jmpV-=3;
          }
        }else{
          if(fallDist>0){
            if(fallDist>36){
              setState(lostLife);
            }
          }
          fallDist=0;
          jmpV=0;
        }

        if(gameFlags&GF_SHOWING_AIR){
          if(air<((levelFlags-2)&0x1f)<<3){
            ++air;
            drawAirBar();
          }else{
            gameFlags&=~GF_SHOWING_AIR;
          }
        }else{
          if((currentFrame%32)==0){
            if(--air==0){
              setState(lostLife);
            }
            drawAirBar();
          }
        }

        if(gameFlags&GF_LEVEL_COMPLETE){
          setState(level==19?gameWon:levelClear);
        }
        
        if(gameFlags&GF_PICKUP_SND){
          sfxV-=10;
          if(sfxV==50){
            gameFlags&=~GF_PICKUP_SND;
            sfxV=0;
          }
        }
      break;

      case levelClear:
        if(gix==0){
          if(air>0){
            score+=5;
            --air;
            sfxV=250-air;
            jmpV=220-air;
            drawAirBar();
            showScore();
          }else{
            levelState=levelLoaded;
            if(++level==20){
              level=0;
            }
            setState(levelStart);
          }
        }
      break;
      
      case lostLife:
      {
        if(gix==0){
          jmpV=0;
          sfxV=0;
          gix=1;
        }
        sfxV+=15;
        jmpV+=12;
        ++giy;
        if(giy>5){
          collapseLevel();
        }
        if(giy==21){
          restoreFontLine=0;
          altFontLine=255;
          clearScreen(4,8,190);
          hideScore();
          if(--lives==0){
            setState(gameOver);
          }else{
            setState(levelStart);
          }
        }
      }
      break;
      
      case gameOver:
      // Show boot drop animation, then return to demo mode...
      // Maybe we'll add a high-score list if enough memory...
      if(gix==0){
        setSpriteDef(0,pgm_read_byte(gameHeader.willyFrameIX+( ((currentFrame>>4)&0x01) ? 2 : 5)) );
        if(++giy==100){
          ++gix;
          giy=0;
          giz=1;
          sfxV=250;
          jmpV=250;
          setSpriteDef(0,pgm_read_byte(gameHeader.willyFrameIX+0));
        }
      }else if(gix==1){
        if(spriteData[2].y<104){
          spriteData[2].y+=giz;
          if((vScroll&0x07)==0){
            screenRam[15+(BYTES_PER_BUFFER_LINE*(21-levelLoadCnt))]=251;
            screenRam[16+(BYTES_PER_BUFFER_LINE*(21-levelLoadCnt))]=252;
            ++levelLoadCnt;
          }
          vScroll-=giz;
          if(++giy==8){
            giy=0;
            if(giz<2){
              giz<<=1;
            }
          }
          setSpritePos(2,120,spriteData[2].y);
          sfxV-=(5*giz);
          jmpV-=2;
        }else{
          jmpV=0;
          sfxV=0;
          drawText(7, 27, gameHeader.gameOverText, 18);
          gix=2;
          giy=0;
        }
      }else if(gix==2){
        if(++giy==50){
          storeHi();
          clearScreen(0,32,190);
          setState(title);
        }
      }
      break;
      
      case gameWon:
      if(gix==0){
        gix=1;
        levelState=levelNotLoaded;
        hideSprites();
        SET_TILE_ANIMATION_OFF;
      }else if(gix==1){
        if(giy++<16){
          collapseLevel();
        }else{
          clearScreen(27,4,190);
          clearScreen(0,3,190);
          gix=2;
        }
      }else if(gix==2){
        vScroll=8; //(PIXELS_PER_CHARACTER*(19+28));
        if(drawTitleGfx()){
          gix=3;
          giz=0;
          cursorY=0;
          for(giy=0;giy<9;giy++){
            setSpriteDef(giy,144+(giy%4));
          }
        }
      }else if(gix==3){
        
        for(giy=0;giy<9;giy++){
          if((currentFrame%((giy>>2)+2))==0){
            if(++spriteData[giy].defIX==148){
              if(giy<4){
                giu=(random(96)&0xf0);
              }else{
                giu=random(96);
              }
              spriteData[giy].x=giu;
              spriteData[giy].y=random(70)+(giy*5);
              spriteData[giy].defIX=144;
            }
          }
        }
        if(giz<111){
          jmpV-=16;
          if(--jmpV<140){
            jmpV=235;
          }
          if((currentFrame%4)==0){
           drawBigText(13,10,giz,gameHeader.congratulationsText);
            ++giz;
          }
        }else{
          jmpV=0;
          if(keyState.changed){
            setState(title);
          }
        }
      }
      break;
    }
    setScroll();
    updateSprites();
    animateTiles();
  }
}
