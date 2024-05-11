#include <Arduino.h>
#line 1 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Scramble\\Scramble\\Scramble.ino"
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

#include "stdint.h"
#include "engine.h"
#include "gameVars.h"
#include "gameFuncs.h"

// The user game functions are all in the gameFuncs.h file. It uses variables found in gameVars.h, and defines in gameDefs.h
// The engine.h, engineDefs.h, engineVars.h and videoOutput.S files are the game engine itself.

#line 27 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Scramble\\Scramble\\Scramble.ino"
void setup();
#line 32 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Scramble\\Scramble\\Scramble.ino"
void loop();
#line 27 "C:\\Users\\ScottPorter\\Documents\\git projects\\ManicMinerForArduinoUNO\\games\\Scramble\\Scramble\\Scramble.ino"
void setup() {
  engineSetup();
  setState(gameInit);
}

void loop() {
  gameLoop();  
}

