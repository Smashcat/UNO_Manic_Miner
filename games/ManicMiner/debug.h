void showDebug(){
  Serial.begin(115200);
  for(uint16_t n=200;n<300;n+=8){
    Serial.print(n>>3);
    Serial.print(" = ");
    Serial.println( 4+((n>>3)%(CHARACTER_ROWS-4)) );  
  }
  while(1);
}

void printBinary(uint8_t x){
  for(int8_t n=7;n>-1;n--){
    Serial.print( x&(1<<n)?"@":".");
  }
}
