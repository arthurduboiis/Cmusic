//
//  dropEvent.h
//  Cmusic
//
//  Created by Arthur dubois on 26/01/2022.
//

#ifndef dropEvent_h
#define dropEvent_h
#include "common.h"
#include "initMedia.h"


typedef struct DropButton{
    int width;
    int height;
    int x;
    int y;
} DropButton;

DropButton drop;

void dropEvent(SDL_Event *e);
void setDropButton(int width, int height, int x, int y);




#endif /* dropEvent_h */
