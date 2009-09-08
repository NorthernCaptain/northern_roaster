#ifndef MYTYPES_H
#define MYTYPES_H

#ifndef uchar
typedef unsigned char uchar;
#endif

#ifndef ulong
typedef unsigned long ulong;
#endif

#ifndef uint
typedef unsigned int uint;
#endif

#define L_MAXPATH   2048


enum Colors 
    {
	BlueColor=54,
	BackColor=55,
	WhiteColor=57,
	DarkRedColor=58,
	WhiteColorAlpha=59,
	YellowColor=60,
	DarkBrownColor=61
    };

/* images and their sets */
enum Images { 
             imNone=-1,
             imBoardBG=0, 
             imIntroBG=1, 
             imCuts=2, 
             imIntro2BG=3, 
             imFont1=4,
             imFinishBG=5,
	     imGameBG=6,
	     imBackTiles1=7,
	     imBackTiles2=8,
	     imMain=9,  //Main window
	     imBackTiles3=10,
	     imBackTiles4=11,
	     imEnemy1=12,
	     imShocker1=13,
	     imShocker2=14,
	     imStatic=15,
	     imTranslator=16,
	     imTeleport=17,
	     imCuts2=18,
	     imLoading=19,
	     imEdable=20,
	     imItems=21,
	     imItemsEmboss=22,
	     imBallFire=23,
	     imFont2=24,
	     imFont3=25,
	     imFont4=26,
	     imFont5=27,

	     imBalls=28,
	     imCursor=29,
	     imSparks=30,
	     imDigits=31,
	     imLoginBG=32,
	     imFont6=33,
	     imConfirmBG=34
};

/* Key and object actions */
enum Actions {
             NOTHING, 
	     FALL, 
	     LEFT, 
	     RIGHT, 
	     UP,
	     DOWN,
	     FIRE,
	     PAUSE_GAME,
             END_GAME,
	     LEVEL_UP,
	     LEVEL_DOWN,
	     TEST_1,
	     TEST_2,
	     FIRE2
};


const int    max_fonts=6;

enum FontType {
    TFont1=0,
    TFont2=1,
    TFont3=2,
    TFont4=3
};

#define MAX_IMAGES        40

/* screen size */
const int MAX_SCREEN_L=      1024;
const int MAX_SCREEN_H=       768;

/* For pixelezation */
const int PIXEL_L=              8;
const int PIXEL_H=              8;

#define MAP_X_TOTAL         16
#define MAX_Y_TOTAL         32

#define MIN2(x,y)   (x < y ? x : y)
#define MAX2(x,y)   (x > y ? x : y)

/* sprite block size */
const int block_x_size=96;
const int block_y_size=72;

/* number of blocks on game field */
const int x_blocks=10;
const int y_blocks=9;

const int game_view_l=block_x_size*x_blocks;
const int game_view_h=block_y_size*y_blocks;

/* game view delta for screen */
const int game_view_x=(MAX_SCREEN_L - block_x_size*x_blocks)/2;
const int game_view_y=(MAX_SCREEN_H - block_y_size*y_blocks)-2;

/* pixel map for block */
const int pixel_map_l=block_x_size/PIXEL_L;
const int pixel_map_h=block_y_size/PIXEL_H;

const int outer_wall_l=PIXEL_L*2;
const int outer_wall_h=PIXEL_H*2;

extern void InitRNG();

/*** random number generator ***/

/* insert your favorite */
extern void SetRNG(long);
extern long LongRNG();
extern void InitRNG();
#define SRAND(X) SetRNG((long) X)
#define LRAND() LongRNG()

#endif
/* ------------ End of file -------------- */

