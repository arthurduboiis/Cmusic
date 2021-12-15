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




_Bool createLeftMenu(void);
_Bool createBottomMenu(void);
_Bool createCenterMenu(void);


void buttonSelected(void);
void setRectSelected(SDL_Rect* size, int y);

LTexture inputTextTexture;
char *textFromInput;

LTexture logoTexture;
LTexture recSelected;
LTexture homeTxt;
LTexture homePng;
LTexture newTxt;
LTexture newPng;
LTexture topTxt;
LTexture topPng;
LTexture playlistTxt;
LTexture playlistPng;
LTexture paramTxt;
LTexture paramPng;

LTexture textTexture;
LTexture mp3Texture;

LTexture background;
LTexture play;
LTexture stop;
LTexture nextRight;
LTexture nextLeft;
LTexture randomMusic;
LTexture replay;
LTexture sound;





SDL_Rect recSelectedSize;



#endif /* viewport_h */
