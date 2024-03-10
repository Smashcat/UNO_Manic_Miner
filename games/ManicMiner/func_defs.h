void playAudio();
void setAudioType(AudioType at);


// funcs.h
void unpackBytes(uint8_t *dst, const uint8_t *conversionTable, const uint8_t *src, uint16_t len, uint8_t bits);
void initHardware();

// game.h
uint8_t getWillyFrame();
void setWillyPos(uint8_t willyX,uint8_t willyY);
void setWillyInitialPos();
boolean loadLevel();
void setState(GameState nextState);
void doScroller();
boolean drawTitleGfx();
uint8_t tileTypeAt(uint16_t ix);
uint8_t getAvailableDirectionFlags();
CollisionType checkCollisions();
void gameLoop();

// input.h
uint8_t readInput();

// sprite.h
void vMoveSprites(int8_t y, boolean forceYDown);
void setSpritePos(uint8_t ix,uint8_t x, uint8_t y);
void hideSprite(uint8_t n);
void hideSprites();
void setSpriteDef(uint8_t ix, uint8_t defIX);
void updateGamePositions(uint8_t n);
void updateSprites();

// tileMap.h
void drawNumber(uint8_t x, uint8_t y, uint16_t num);
void drawBigNumber(uint8_t x,uint8_t y,uint16_t num);
void drawLives();
void drawText(uint8_t x, uint8_t y, uint8_t *txt, uint8_t len);
void setTileRowSplit(uint16_t x);
void setScroll();
void clearScreen(uint8_t startRow, uint8_t numRows, uint8_t tileIX);
void drawOutline(uint8_t startRow,uint8_t height,uint8_t isOn);
void showScore();
void hideScore();
void drawAirBar();
void drawAirBarUI();
void collapseLevel();
void animateTiles();
