// Update input hold buffer. If something has changed, returns bitfield of changed buttons
// Note that a LOW bit = button pressed
// Order of bits from LSb->MSb = "B","A","Down","Up","Right","Left","Start","Option" (Start not yet connected)
// First 6 inputs are on PORTC, Option on PORTD2, Start on PORTD0
uint8_t readInput(){
  keyState.last=keyState.current;
  keyState.current=PINC&0x3f;
  keyState.current|=((PIND&(1<<2))<<4); // Option button on PORTD
  keyState.current|=((PIND&1)<<7);      // Start button on PORTD
  keyState.changed=(keyState.current^keyState.last);
  return keyState.changed;
}
