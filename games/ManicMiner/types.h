
typedef bool boolean;

typedef struct ProcessedSprite{
  uint8_t yLine;        // Scanline at top of sprite
  uint8_t xByte;        // Byte position in line buffer
  uint8_t sData[3*16]; // Graphical data, pre-shifted ready to display on scanlines
} ProcessedSprite;

typedef struct Pos{
  uint8_t y;
  uint8_t x;
} Pos;

typedef struct SpriteData{
  uint8_t y;
  uint8_t x;
  uint8_t defIX;
} SpriteData;

typedef struct Guardian{
  const uint8_t flags;
  const uint8_t startPos[2];
  const uint8_t maxPos;
  const uint8_t minPos;  
} Guardian;

typedef struct GameLevel{
  const uint8_t   map[192];
  const uint8_t   tileIX[9];
  const uint8_t   portalPos[2];
  const uint8_t   pickupPos[2*5];
  const uint8_t   levelName[20];
  const uint8_t   guardianFrameIX[8];
  const uint8_t   willyInit[2];
  const uint8_t   flags;
  const Guardian  guardian[8];
} GameLevel;

typedef struct KeyState{
  uint8_t current;
  uint8_t last;
  uint8_t changed;
} KeyState;

typedef enum AudioType{
  muted,
  titleMusic,
  gameMusic,
  noMusic
} AudioType;

typedef enum GameState{
  idle,
  title,
  levelStart,
  playing,
  levelClear,
  lostLife,
  gameOver,
  gameWon
} GameState;

typedef enum LevelState{
  levelNotLoaded,
  levelHold,
  levelLoading,
  levelLoaded,
  levelPositioning,
  levelPositioning2,
  levelPositioning3,
  levelReady
} LevelState;

typedef enum CollisionType{
  hitNone,
  hitNasty,
  hitPickup,
  hitPortal,
  hitSwitch
} CollisionType;
