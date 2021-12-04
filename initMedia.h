//
//  initMedia.h
//  Cmusic
//
//  Created by Arthur dubois on 29/11/2021.
//

#ifndef initMedia_h
#define initMedia_h

#include "common.h"

#define SCREEN_WIDTH 1152
#define SCREEN_HEIGHT 648
#define FONT_SIZE 32


SDL_Renderer *renderer;
SDL_Window *window;
TTF_Font *gFont;
char mot[100][100];

typedef struct LTexture {
    SDL_Texture *mTexture;
    int mWidth;
    int mHeight;
} LTexture;

_Bool init(void);
void closee(void);

_Bool loadMediaText(void);

#if defined(SDL_TTF_MAJOR_VERSION)
_Bool loadFromRenderedText(LTexture *ltexture, char textureText[]);
#endif


_Bool loadFromFile(char path[],LTexture *finalTexture);
void freeLtexture(LTexture *texture);
void renderLTexture(LTexture texture,  int x, int y, SDL_Rect *clip);


SDL_Texture* loadMediaTexture(char path[]);



//Gets LTexture dimensions
int getWidth(LTexture *texture);
int getHeight(LTexture *texture);

#endif /* initMedia_h */
