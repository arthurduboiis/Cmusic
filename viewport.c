//
//  viewport.c
//  Cmusic
//
//  Created by Arthur dubois on 02/12/2021.
//

#include "viewport.h"

LTexture logoTexture;


_Bool createLeftMenu(void)
{
    
    _Bool success = 1;
    
    SDL_Rect viewportSize;
    viewportSize.h = 568;
    viewportSize.w = 200;
    viewportSize.x = 0;
    viewportSize.y = 0;
    SDL_RenderSetViewport( renderer, &viewportSize );
    
    SDL_SetRenderDrawColor( renderer, 89, 106, 255, 0xFF );
    SDL_RenderFillRect( renderer, &viewportSize );
    //LTexture logoTexture;
    if(!loadFromFile("./Ressources/img/logo_cmusic.png", &logoTexture)){
        printf( "Failed to load button sprite texture!\n" );
        success = 0;
        return success;
    }

    renderLTexture(logoTexture, 50, 15, NULL);
    
    
    
    
    return success;
    
}
