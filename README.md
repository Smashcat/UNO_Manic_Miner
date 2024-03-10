# Arduino UNO Manic Miner
Manic Miner on the Arduino UNO with composite output

This is a project to demonstrate the use of my game engine, with a port of Manic Miner, originally for the ZX Spectrum, by Matthew Smith. This is currently in progress.


## Engine Features:

+ Composite output using 2 resistors
+ Scrolling tilemap background
+ Up to 9 16x16 sprites on screen, with up to 7 per scanline
+ 1 sprite has pixel-perfect collision detection against other sprites
+ Fixed framerate of 50fps
+ Effective esolution is 256x256 (no framebuffer, everything is generated by "racing the beam"
+ Lots of spare cycles, enough to run game logic and easily update sprite positions, and even all tile indexes on screen every frame!


## PCB Features:
  
+ PCB design schematics for a shield for the Arduino UNO, with buttons, composite output, speaker driving circuit and a an area to mount a Nintendo NES controller port to use instead of the buttons
+ All files for PCB, including production files ready to place a fully assembled order (except for the NES port and header pins) from JLCPCB. Cost including delivery per board for 20 fully assembled boards seems to be around £4 for me in the UK.


## Game Features:

+ All 20 original levels
+ All "special" boss and other mechanics. Including Eugene, the Kong Beast, Skylab crashing enemies and Solar Generator beam
+ Extra animation on final cavern level showing realtime tile animation
+ Menu screen with options to choose start level, turn music on/off and reset high score which is persistant between boots
+ 2 channel audio with original music
+ Special end screen and message if you complete the game!


## YouTube:
 
[![Manic Miner on Arduino UNO](https://img.youtube.com/vi/dm_dUSeIqC4/0.jpg)](https://www.youtube.com/watch?v=dm_dUSeIqC4)


## Screen-shots:

![Render](./images/1.jpg?raw=true)

![Render](./images/2.jpg?raw=true)

![Render](./images/3.jpg?raw=true)

![Render](./images/4.jpg?raw=true)

![Render](./images/5.jpg?raw=true)


