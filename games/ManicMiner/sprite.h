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

extern "C" { void processSprite(ProcessedSprite *dest, const uint8_t *spriteDefRef, const SpriteData *spriteData); }

void vMoveSprites(int8_t y, boolean forceYDown){
  for(uint8_t n=0;n<NUM_SPRITES;n++){
    if(spriteData[n].x<255){
      if(forceYDown && (verticalGuardian&(1<<n))){
        spriteMin[n]+=y;
        if(spriteMax[n]<245){
          spriteMax[n]+=y;
        }
      }
      spriteData[n].y+=y;
      if(spriteData[n].y>210 && spriteData[n].y<245){
        spriteData[n].x=255;
        spriteData[n].y=n*16; // If too many sprites on one line, we lose sync!
      }
    }
  }
}

void setSpritePos(uint8_t ix,uint8_t x, uint8_t y){
  spriteData[ix].y=y;
  spriteData[ix].x=x;
  processSprite(&processedSprite[ix], spriteDef, &spriteData[ix]);  
}

void hideSprite(uint8_t n){
  spriteDir[n]=0;
  spriteMax[n]=255;
  spriteMin[n]=255;
  setSpritePos(n,255,n*20);
}

void hideSprites(){
  for(uint8_t n=0;n<NUM_SPRITES;n++){
    hideSprite(n);
  }
}

void setSpriteDef(uint8_t ix, uint8_t defIX){
  spriteData[ix].defIX=defIX;
}

void updateGamePositions(uint8_t n){
  
  boolean canMove=false;
  boolean isVert=verticalGuardian&(1<<n);
  int8_t spd=spriteDir[n];
  int8_t dir=0;
  if(spd<0){
    dir=-(isVert?1:2);
    spd=-spd;
  }else if(spd>0){
    dir=(isVert?1:2);
  }
  if(isVert){
    canMove=((currentFrame%(5-spd))==0);
  }else{
    canMove=((currentFrame%((3-spd)<<2))==0);
  }

  boolean hasVertical=(willyFlags[1]&WF_HAS_VERT_GUARDIANS);

  uint8_t c;

  // Skylab movement and animation is different to other levels.
  // Sprites move from their minimum position to maximum, vertically, staying on animation frame 0
  // and when hitting maximum, change animation frames from 0 through 7 while stopped. Then move
  // 32 pixels right, and back up to their start minimum positions. They wrap around from right to left.
  if(IS_SKYLAB_LEVEL){
    if(spriteData[n].x==255){
      return;
    }
    if(spriteData[n].y<spriteMax[n]+1){
      if(!canMove){
        return;
      }
      ++spriteData[n].y;
    }else{
      if(spriteFrame[n]<(7<<2)){
        ++spriteFrame[n];
      }else{
        spriteFrame[n]=0;
        spriteData[n].y=spriteMin[n];
        spriteData[n].x+=64;
      }
    }
    setSpriteDef(n,spriteFrameIX[spriteFrame[n]>>2]);
  }else{
    if(!canMove){
      return;
    }
    if(isVert){
      c=spriteData[n].y;
    }else{
      c=spriteData[n].x;
    }
    c+=dir;
    if(dir>0 && c>spriteMax[n]){
      c=spriteMax[n];
      spriteDir[n]=-spriteDir[n];
    }else if(dir<0 && c<spriteMin[n]){
      c=spriteMin[n];
      spriteDir[n]=-spriteDir[n];
    }
    if(isVert){
      spriteData[n].y=c;
    }else{
      spriteData[n].x=c;
    }
    
    if(eugeneGuardian&(1<<n)){
      // No animation;
      if((gameFlags&GF_PORTAL_READY) && ((gameFlags&GF_EUGENE_ACTIVE)==0)){
        gameFlags|=GF_EUGENE_ACTIVE;
      }
      if(gameFlags&GF_EUGENE_ACTIVE){
        spriteDir[n]=1;
        uint8_t yEnd=((13*8)-vScroll);
        if(spriteData[n].y>yEnd){
          spriteData[n].y=yEnd;
        }
        spriteData[n].x=((currentFrame>>2)&1)?255:(15*8);
      }
    }else if(kongGuardian&(1<<n)){
      if(gameFlags&GF_KONG_SWITCH_2){
        setSpriteDef(n,spriteFrameIX[((currentFrame>>4)&0x01)+2]);
        spriteDir[n]=4;
        uint8_t yEnd=((13*8)-vScroll)-25;
        if(spriteData[n].y>=yEnd){
          spriteDir[n]=0;
          spriteData[n].y=yEnd;
          spriteData[n].x=255;
        }
      }else{
        setSpriteDef(n,spriteFrameIX[((currentFrame>>4)&0x01)]);
      }
    }else{
      boolean isVert=(verticalGuardian&(1<<n));
      if(hasVertical){
        if(isVert){
          if(++spriteFrame[n]==4){
            spriteFrame[n]=0;
          }
        }else{
          if(spriteDir[n]<0){
            if(--spriteFrame[n]<4){
              spriteFrame[n]=7;
            }
          }else if(spriteDir[n]>0){
            if(spriteFrame[n]<4){
              spriteFrame[n]+=4;
            }
            if(++spriteFrame[n]==8){
              spriteFrame[n]=4;
            }
          }
        }
      }else{
        if(spriteDir[n]>0){
          if(++spriteFrame[n]>3){
            spriteFrame[n]=0;
          }
        }else if(spriteDir[n]<0){
          if(spriteFrame[n]<4){
            spriteFrame[n]+=4;
          }
          if(++spriteFrame[n]>7){
            spriteFrame[n]=4;
          }
        }
      }
      setSpriteDef(n,spriteFrameIX[spriteFrame[n]]);
    }
  }
}

void updateSprites(){
      
  for(uint8_t n=0;n<NUM_SPRITES-1;n++){
    if(levelState==levelReady){
      updateGamePositions(n);
    }    
    setSpritePos(n,spriteData[n].x,spriteData[n].y);
  }
}
