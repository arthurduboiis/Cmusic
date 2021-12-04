//
//  render.c
//  Cmusic
//
//  Created by Arthur dubois on 01/12/2021.
//

#include "render.h"

void clearRenderer(void){
    SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    SDL_RenderClear( renderer );
}


