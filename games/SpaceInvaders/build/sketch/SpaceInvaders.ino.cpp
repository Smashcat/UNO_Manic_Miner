#include <Arduino.h>
#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Space Invaders\\SpaceInvaders\\SpaceInvaders.ino"
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

/**
 * 
 * Original game by Matthew Smith for ZX Spectrum 48K in 1983. 
 * 
 * Thanks to ceptimus on YouTube for information about the correct timings for the sync pulses!
 * 
 */

#include "stdint.h"
#include "engine.h"
#include "gameVars.h"
#include "debug.h"
#include "game.h"


#line 34 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Space Invaders\\SpaceInvaders\\SpaceInvaders.ino"
void setup();
#line 39 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Space Invaders\\SpaceInvaders\\SpaceInvaders.ino"
void loop();
#line 34 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Space Invaders\\SpaceInvaders\\SpaceInvaders.ino"
void setup() {
  engineSetup();
  setState(gameInit);
}

void loop() {
  gameLoop();  
}

