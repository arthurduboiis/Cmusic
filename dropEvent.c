//
//  dropEvent.c
//  Cmusic
//
//  Created by Arthur dubois on 26/01/2022.
//

#include "dropEvent.h"


void dropEvent(SDL_Event* e)
{
    
    
    char* dropped_filedir;
    
    int xMouse,yMouse;
    SDL_GetMouseState(&xMouse,&yMouse);
    
    _Bool inside = 1;
    
    if (xMouse < drop.x || xMouse > drop.x + drop.width || yMouse < drop.y || yMouse > drop.y + drop.height)
    {
        inside = 0;
    }
    
    else {
        if( e->type == SDL_DROPFILE){
            dropped_filedir = e->drop.file;
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                "Le fichier a été envoyé au serveur.",
                dropped_filedir,
                window
            );
            SDL_free(dropped_filedir);
        }
    }
}


void setDropButton(int width, int height, int x, int y)
{
    drop.width = width;
    drop.height = height;
    drop.x = x;
    drop.y = y;
}
