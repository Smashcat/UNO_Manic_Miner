void playAudio(){
  
  if(audioType==titleMusic){
    if(--musicTmr==0){
      musicTmr=pgm_read_byte(gameHeader.titleMusic[0]+musicPos);
      if(IS_MUSIC_ON){
        OCR2A=pgm_read_byte(gameHeader.titleMusic[1]+musicPos);
        OCR0A=pgm_read_byte(gameHeader.titleMusic[2]+musicPos);
      }else{
        OCR0A=0;
        OCR2A=0;
      }
      if(++musicPos>=95){
        musicPos=0;
      }
    }

  }else if(audioType==gameMusic){
    if(--musicTmr==0){
      musicTmr=resetMusicTmr;
      if(++musicFF==4){
        if(++musicPos>=64){
          musicPos=0;
        }
        musicFF=0;
      }
      uint8_t v=pgm_read_byte(gameHeader.inGameMusic+musicPos);
      if(IS_MUSIC_ON){
        OCR0A=((musicFF%2)==0?v:0);
      }else{
        OCR0A=0;
      }
    }
    if(sfxV>0){
      OCR2A=sfxV;
    }else if(jmpV>0){
      OCR2A=jmpV;
    }else{
      OCR2A=0;
    }
    
  }else if(audioType==noMusic){
    OCR0A=sfxV;
    OCR2A=jmpV;
  }
  
}

void setAudioType(AudioType at){
  jmpV=0;
  sfxV=0;
  if(at==audioType){
    return;
  }
  OCR0A=0;
  OCR2A=0;
  if(at!=muted){
    UNMUTE_PWM_1;
    UNMUTE_PWM_2;
  }else{
    MUTE_PWM_1;
    MUTE_PWM_2;
  }
  if(at==titleMusic){
    TCCR0B = _BV(WGM22) | _BV(CS01) | _BV(CS00);
    TCCR2B = _BV(WGM22) | _BV(CS22);
  }else{
    TCCR0B = _BV(WGM22) | _BV(CS02);
    TCCR2B = _BV(WGM22) | _BV(CS21) | _BV(CS20);
  }
  audioType=at;
  musicPos=0;
  musicTmr=2;
  musicFF=0;
}
