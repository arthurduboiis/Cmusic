//
//  mouseButton.h
//  Cmusic
//
//  Created by Arthur dubois on 08/12/2021.
//

#ifndef mouseButton_h
#define mouseButton_h

#include "common.h"
#include "initMedia.h"
#include "viewport.h"

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50
#define TOTAL_BUTTONS_LEFT_MENU 5
#define X_BUTTON_LEFT_MENU 0
#define Y_BUTTON_HOME 193
#define Y_BUTTON_NEW 243
#define Y_BUTTON_RANK 293
#define Y_BUTTON_PLAYLIST 343
#define Y_BUTTON_SETTING 518

#define BUTTON_BOTTOM_WIDTH 25
#define BUTTON_BOTTOM_HEIGHT 25

#define X_BUTTON_SHUFFLE 423
#define X_BUTTON_SKIP_LEFT 508
#define X_BUTTON_PLAY 553
#define X_BUTTON_SKIP_RIGHT 598
#define X_BUTTON_REPEAT 683
#define Y_BUTTON_BOTTOM_MENU 595.5

#define BUTTON_VOLUME_SIZE 16

#define TOTAL_ADD_BUTTONS 3

#define SCROLLING_AREA_WIDTH 932
#define SCROLLING_AREA_HEIGHT 150

#define TOTAL_BUTTON_SCROLLING_AREA 12
#define SCROLLING_BUTTON_MUSIC_WIDTH 100
#define SCROLLING_BUTTON_MUSIC_HEIGHT 120


typedef enum LButtonEvent
{
    BUTTON_MOUSE_OUT = 0,
    BUTTON_MOUSE_MOTION = 1,
    BUTTON_MOUSE_DOWN = 2,
    BUTTON_MOUSE_UP = 3,
    WHEEL_UP = 4,
    WHEEL_DOWN = 5,
    MOUSE_ON = 6,
    BUTTON_TOTAL = 7
}LButtonEvent;


typedef struct LButton{
    SDL_Point mPosition;
    LButtonEvent mCurrentButton;
}LButton;

LButton gButtonsLeftMenu[ TOTAL_BUTTONS_LEFT_MENU];
LButton gButtonsBottomMenu[ TOTAL_BUTTONS_LEFT_MENU];
LButton volumeButton, scrollingArea, scrollingButton[ TOTAL_BUTTON_SCROLLING_AREA];

int xScrolling;

LButton wantInputButtons[TOTAL_ADD_BUTTONS];

extern LTexture recSelected;
extern SDL_Rect recSelectedSize;
extern LTexture point;


void initLButton(LButton *button);
void setPosition( LButton *button, int x, int y );
void handleEvent( LButton *button, SDL_Event* e, int width, int height );
void initButtonMenu(SDL_Event*);
void dragButtonVolume(SDL_Event*);
void scrollingEvent();
void playButton();
void chooseButton();
void checkRR();
void setPositionButtonLeftMenu(void);
void setPositionButtonBottomMenu(void);
void setPositionScrollingArea(void);
void setPositionButtonViewportNew(void);

#endif /* mouseButton_h */
