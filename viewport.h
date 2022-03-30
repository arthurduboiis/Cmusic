//
//  viewport.h
//  Cmusic
//
//  Created by Arthur dubois on 02/12/2021.
//

#ifndef viewport_h
#define viewport_h


#include "common.h"
#include "render.h"
#include "initMedia.h"
#include "mouseButton.h"
#include "dropEvent.h"
#include "inputText.h"


SDL_Rect inputText;


_Bool createLeftMenu(void);
_Bool createBottomMenu(void);
_Bool createCenterMenu(void);
void createViewportTop(void);
void createViewportNew(void);
void createViewportPlaylist(void);
void createViewportSetting(void);

void createMusicDisplay(LTexture img,LTexture title, LTexture artiste, LTexture musicDuration);

void buttonSelected(void);
void setRectSelected(SDL_Rect* size, int y);


void setXVolume(int);
void setXScrolling(int);

int getSelectedPage(void);

SDL_Rect recSelectedSize;

int spacing;



#endif /* viewport_h */
