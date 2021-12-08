//
//  viewport.c
//  Cmusic
//
//  Created by Arthur dubois on 02/12/2021.
//

#include "viewport.h"

int selectedButton = 0;

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
        printf( "Failed to load ./Ressources/img/logo_cmusic.png \n" );
        success = 0;
        return success;
    }
    
    renderLTexture(logoTexture, 50, 15, NULL);
    
    recSelectedSize.h = BUTTON_HEIGHT;
    recSelectedSize.w = BUTTON_WIDTH;
    recSelectedSize.x = 0;
    recSelectedSize.y = 0;
    
    if(!loadFromFile("./Ressources/img/selected.png", &recSelected)){
        printf("Failed to load ./Ressources/img/selected.png \n");
        success = 0;
        return success;
    }
    buttonSelected();
    setRectSelected(&recSelectedSize, gButtonsLeftMenu[selectedButton].mPosition.y);
    
    loadFromRenderedText(&homeTxt, "Accueil");
    renderLTexture(homeTxt, 62, 207, NULL);
    loadFromFile("./Ressources/img/home.png", &homePng);
    homePng.mHeight = 25;
    homePng.mWidth = 25;
    renderLTexture(homePng, 20,202, NULL);
    
    loadFromRenderedText(&newTxt, "Nouveautés");
    renderLTexture(newTxt, 62, 257, NULL);
    loadFromFile("./Ressources/img/new.png", &newPng);
    newPng.mHeight = 25;
    newPng.mWidth = 25;
    renderLTexture(newPng, 20, 256, NULL);
    
    loadFromRenderedText(&topTxt, "Top");
    renderLTexture(topTxt, 62, 307, NULL);
    loadFromFile("./Ressources/img/rank.png", &topPng);
    topPng.mHeight = 25;
    topPng.mWidth = 25;
    renderLTexture(topPng, 20, 306, NULL);
    
    loadFromRenderedText(&playlistTxt, "Playlists");
    renderLTexture(playlistTxt, 62, 357, NULL);
    loadFromFile("./Ressources/img/playlist.png", &playlistPng);
    playlistPng.mHeight = 25;
    playlistPng.mWidth = 25;
    renderLTexture(playlistPng, 20, 356, NULL);
    
    loadFromRenderedText(&paramTxt, "Paramètres");
    renderLTexture(paramTxt, 62, 532, NULL);
    loadFromFile("./Ressources/img/setting.png", &paramPng);
    paramPng.mHeight = 25;
    paramPng.mWidth = 25;
    renderLTexture(paramPng, 20, 531, NULL);
    
   
    
    return success;
    
}

void buttonSelected(void){
    int i;
    for(i=0; i < TOTAL_BUTTONS_LEFT_MENU; i++){
        if( gButtonsLeftMenu[i].mCurrentButton == 3 ){
            selectedButton = i;
        }
    }
}

void setRectSelected(SDL_Rect* size, int y)
{
    if(!loadFromFile("./Ressources/img/selected.png", &recSelected)){
        printf("Failed to load ./Ressources/img/selected.png \n");
        
    }
    
    renderLTexture(recSelected, X_BUTTON_LEFT_MENU,y, size);
    
}
