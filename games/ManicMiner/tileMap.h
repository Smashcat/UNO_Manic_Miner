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

void drawNumber(uint8_t x, uint8_t y, uint16_t num){
  uint8_t *p=screenRam+(x+(y*BYTES_PER_BUFFER_LINE))+5;
  uint8_t fontIX=pgm_read_byte(&gameHeader.fontIX);
  for(uint8_t n=0;n<6;n++){
    *p--=(fontIX+(num%10));
    num/=10;
  }
}

void drawBits(uint8_t x, uint8_t y, uint8_t num){
  uint8_t *p=screenRam+(x+(y*BYTES_PER_BUFFER_LINE))+8;
  uint8_t fontIX=pgm_read_byte(&gameHeader.fontIX);
  for(uint8_t n=0;n<8;n++){
    *p--=(fontIX+(num&1));
    num>>=1;
  }
}

void drawBigNumber(uint8_t x,uint8_t y,uint16_t num){
  uint8_t *p=screenRam+(x+(y*BYTES_PER_BUFFER_LINE))+(5*2);
  uint8_t fontIX=pgm_read_byte(&gameHeader.bigFontIX);
  // bigFontMap
  for(uint8_t n=0;n<6;n++){
    uint8_t d=(num%10)<<2;
    num/=10;
    const uint8_t *s=gameHeader.bigFontMap+d;
    *p++=fontIX+pgm_read_byte(s++);
    *p++=fontIX+pgm_read_byte(s++);
    *(p+BYTES_PER_BUFFER_LINE-2)=fontIX+pgm_read_byte(s++);
    *(p+BYTES_PER_BUFFER_LINE-1)=fontIX+pgm_read_byte(s);
    p-=4;
  }
}

void drawLives(){
  uint8_t *p=screenRam+((28*BYTES_PER_BUFFER_LINE)+13);
  uint8_t fontIX=pgm_read_byte(&gameHeader.bigFontIX);
  uint8_t ix=fontIX+32+(((currentFrame>>3)&0x01)*4);
  for(uint8_t n=0;n<lives-1;n++){
    *p++=ix;
    *p++=ix+1;
    *(p+BYTES_PER_BUFFER_LINE-2)=ix+2;
    *(p+BYTES_PER_BUFFER_LINE-1)=ix+3;
  }
}

void drawText(uint8_t x, uint8_t y, const uint8_t *txt, uint8_t len){
  uint8_t *p=screenRam+(x+(y*BYTES_PER_BUFFER_LINE));
  uint8_t fontIX=pgm_read_byte(&gameHeader.fontIX);
  while(len--){
    *p++=(pgm_read_byte(txt++)+fontIX);
  }
}

void setTileRowSplit(uint16_t x){
  lineCompare=x;
  tileRowStart=CHARACTER_ROWS-(lineCompare/PIXELS_PER_CHARACTER);
}

void setScroll(){
  uint8_t a=(uint8_t)(hScroll&0x07);
  hScrollSMask=0x7f>>(7-a);
  hScrollEMask=0xfe<<a;
}

void drawHLine(uint8_t *p,uint8_t len,uint8_t tileIX){
  while(len--){*p++=tileIX;}
}

void drawVLine(uint8_t *p,uint8_t len,uint8_t tileIX){
  while(len--){*p=tileIX;p+=BYTES_PER_BUFFER_LINE;}
}

void clearScreen(uint8_t startRow, uint8_t numRows,uint8_t tileIX){
  uint8_t *p=screenRam+(startRow*BYTES_PER_BUFFER_LINE);
  while(numRows--){
    drawHLine(p,BYTES_PER_BUFFER_LINE,tileIX);
    p+=BYTES_PER_BUFFER_LINE;
  }
}


void drawOutline(uint8_t startRow,uint8_t height,uint8_t isOn){
  uint8_t *p=screenRam+(startRow*BYTES_PER_BUFFER_LINE);
  clearScreen(startRow,height+2,190);
  if(isOn){
    drawHLine(p,BYTES_PER_RASTER,199);
    drawHLine(p+(BYTES_PER_BUFFER_LINE*(height+1)),BYTES_PER_RASTER,200);
    drawVLine(p,height+2,201);
    drawVLine(p+(BYTES_PER_RASTER-1),height+2,202);
    *p=195;                     // Top left
    *(p+31)=196;                // Top right
    *(p+(32+(32*height)))=197;  // Bottom left
    *(p+31+32+(32*height))=198; // Bottom right
  }
}

void showScore(){
  if(score>hiScore){
    hiScore=score;
  }
  drawBigNumber(1,28,score);
  drawBigNumber(19,28,hiScore);
}

void hideScore(){
  clearScreen(27,4,190);  
}

void drawAirBar(){
  uint8_t *p=screenRam+(1*BYTES_PER_BUFFER_LINE)+5;
  uint8_t airIX=pgm_read_byte(&gameHeader.airIX);
  uint8_t len=(air>>3);
  for(uint8_t n=0;n<26;n++){
    *(p+n)=(len>n?airIX:190);
  }
  if(len<26 && air&0x07){
    *(p+len)=(airIX+(air&0x07));
  }
}

void drawAirBarUI(){
  drawOutline(0,1,1);
  uint8_t *p=screenRam+(1*BYTES_PER_BUFFER_LINE)+1;
  uint8_t fontIX=pgm_read_byte(&gameHeader.fontIX);
  *p++=(fontIX+12+'A'-'A');
  *p++=(fontIX+12+'I'-'A');
  *p++=(fontIX+12+'R'-'A');
}

void collapseLevel(){
  for(uint8_t y=4;y<32;y++){
    uint8_t *p=screenRam+(y*BYTES_PER_BUFFER_LINE);
    uint8_t b=(level==19 &&y<12)?3:190;
    for(uint8_t x=16;x>0;x--){
      *(p+x)=*(p+x-1);
    }
    *p=b;

    for(uint8_t x=16;x<31;x++){
      *(p+x)=*(p+x+1);
    }
    *(p+31)=b;

  }
}

void clearSolarLight(){
  uint8_t tIX=pgm_read_byte(gameLevel[level].tileIX); // Index of background tile in font
  uint8_t *p=screenRam+(4*BYTES_PER_BUFFER_LINE)+1;
  for(uint16_t n=0;n<512;n++){
    if(*p==254){
      *p=tIX;    
    }
    ++p;
  }
}

void drawSolarLight(){
  uint8_t tIX=pgm_read_byte(gameLevel[level].tileIX);
  uint8_t x=23;
  uint8_t y=4;
  uint8_t dir=0;  // 0=down, 1=left
  uint8_t *p;
  while(1){
    p=screenRam+((y*BYTES_PER_BUFFER_LINE)+x);
    uint8_t lX=x<<3;
    uint8_t lY=y<<3;
    for(uint8_t n=0;n<8;n++){
      uint8_t sX=spriteData[n].x+2;
      uint8_t sY=spriteData[n].y-7;
      if(!(lX>sX+15 || lX<sX-7 || lY<sY-7 || lY>sY+15)){
        dir=(dir==0?1:0);
      }
    }
    if(*p==tIX){
      *p=254;
      if(dir==0){
        ++y;
      }else if(dir==1){
        --x;
      }
    }else{
      break;
    }
  }
}

void animateTiles(){
  if(!DO_TILE_ANIMATION){
    return;
  }
  uint8_t cS=pgm_read_byte(&gameHeader.conveyorFloorIX);
  uint8_t cE=pgm_read_byte(&gameHeader.tileIX);
  
  uint8_t *p=screenRam+(BYTES_PER_BUFFER_LINE*(4+(level==19?8:0)));
  uint16_t len=(BYTES_PER_BUFFER_LINE*(level==19?8:16));
//  for(uint16_t n=0;n<len;n++){
  while(len--){
    if(*p<cE && *p>=cS){
      uint8_t a=( (*p) + (levelFlags&(1<<5)?-1:1) )&0x07;
      *p=((*p&0xf8)+a);
    }
    ++p;
  }

  // Don't animate pickups unless the level is loaded
  if(levelState==levelReady){
    
    uint8_t pT=pgm_read_byte(gameLevel[level].tileIX+8);
    uint8_t bT=pgm_read_byte(gameLevel[level].tileIX+0);
    uint8_t ascii=pgm_read_byte(&gameHeader.fontIX);
    uint8_t keysLeft=0;
    
    for(uint8_t n=0;n<5;n++){
      if(pickup[n]){
        ++keysLeft;
        if(screenRam[pickup[n]]==ascii+39){
          screenRam[pickup[n]]=bT;
          pickup[n]=0;
        }else if(screenRam[pickup[n]]==255){
          screenRam[pickup[n]]=3;
          pickup[n]=0;
        }else{
          if( (level==19) && (pickup[n]<(12*BYTES_PER_BUFFER_LINE)) ){
            screenRam[pickup[n]]=((((currentFrame+(n*2))%16)<4)?3:63);
          }else{
            screenRam[pickup[n]]=((((currentFrame+(n*2))%16)<4)?bT:pT);
          }
        }
      }
    }

    // if no keys left, animate the exit
    uint8_t pX=pgm_read_byte(gameLevel[level].portalPos);
    uint8_t pY=pgm_read_byte(gameLevel[level].portalPos+1)+4;
    uint16_t xy=((pY*BYTES_PER_BUFFER_LINE)+pX);
    uint8_t portalIX=pgm_read_byte(&gameHeader.portalIX);
    uint8_t off=0;
#ifndef DISABLE_COLLECT_PICKUPS
    if(keysLeft==0){
      gameFlags|=GF_PORTAL_READY;
      off=(currentFrame>>2)&(1<<2);
    }
#else
      gameFlags|=GF_PORTAL_READY;
      off=(currentFrame>>2)&(1<<2);
#endif
    if(level==19){
      portalIX=55;
    }
    screenRam[xy]=portalIX+off;
    screenRam[xy+1]=portalIX+off+1;
    screenRam[xy+BYTES_PER_BUFFER_LINE]=portalIX+off+2;
    screenRam[xy+BYTES_PER_BUFFER_LINE+1]=portalIX+off+3;

    if(IS_SOLAR_POWER_LEVEL){
      clearSolarLight();
      drawSolarLight();
    }

    if(level==19){
      const uint8_t *p=gameHeader.sunLayout+(sunFrame*32);

      for(uint8_t y=0;y<4;y++){
        for(uint8_t x=0;x<8;x++){
          if(!(y==0 && (x==0 || x==7))){
            screenRam[((6+y)*BYTES_PER_BUFFER_LINE)+(5+x)]=pgm_read_byte(p)+64;
          }
          ++p;
        }
      }

      if(currentFrame%2){
        if(++sunFrame==6){
          sunFrame=0;
        }
      }
    }
  }
}
