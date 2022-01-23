//
//  viewport.c
//  Cmusic
//
//  Created by Arthur dubois on 02/12/2021.
//

#include "viewport.h"

int selectedButton = 0;
int xVolume = 1038;


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
    if(!loadFromFile(CHEMIN"Ressources/img/logo_cmusic.png", &logoTexture)){
        printf( "Failed to load" CHEMIN "Ressources/img/logo_cmusic.png \n" );
        success = 0;
        return success;
    }
    
    renderLTexture(logoTexture, 50, 15, NULL);
    
    recSelectedSize.h = BUTTON_HEIGHT;
    recSelectedSize.w = BUTTON_WIDTH;
    recSelectedSize.x = 0;
    recSelectedSize.y = 0;
    
    if(!loadFromFile(CHEMIN"Ressources/img/selected.png", &recSelected)){
        printf("Failed to load" CHEMIN "Ressources/img/selected.png \n");
        success = 0;
        return success;
    }
    buttonSelected();
    setRectSelected(&recSelectedSize, gButtonsLeftMenu[selectedButton].mPosition.y);
    
    loadFromRenderedText(&homeTxt, "Accueil");
    renderLTexture(homeTxt, 62, 207, NULL);
    loadFromFile(CHEMIN"Ressources/img/home.png", &homePng);
    homePng.mHeight = 25;
    homePng.mWidth = 25;
    renderLTexture(homePng, 20,202, NULL);
    
    loadFromRenderedText(&newTxt, "Nouveautés");
    renderLTexture(newTxt, 62, 257, NULL);
    loadFromFile(CHEMIN"Ressources/img/new.png", &newPng);
    newPng.mHeight = 25;
    newPng.mWidth = 25;
    renderLTexture(newPng, 20, 256, NULL);
    
    loadFromRenderedText(&topTxt, "Top");
    renderLTexture(topTxt, 62, 307, NULL);
    loadFromFile(CHEMIN"Ressources/img/rank.png", &topPng);
    topPng.mHeight = 25;
    topPng.mWidth = 25;
    renderLTexture(topPng, 20, 306, NULL);
    
    loadFromRenderedText(&playlistTxt, "Playlists");
    renderLTexture(playlistTxt, 62, 357, NULL);
    loadFromFile(CHEMIN"Ressources/img/playlist.png", &playlistPng);
    playlistPng.mHeight = 25;
    playlistPng.mWidth = 25;
    renderLTexture(playlistPng, 20, 356, NULL);
    
    loadFromRenderedText(&paramTxt, "Paramètres");
    renderLTexture(paramTxt, 62, 532, NULL);
    loadFromFile(CHEMIN"Ressources/img/setting.png", &paramPng);
    paramPng.mHeight = 25;
    paramPng.mWidth = 25;
    renderLTexture(paramPng, 20, 531, NULL);
    
    switch (selectedButton) {
        case 0:
            createCenterMenu();
            break;
        case 1:
            createViewportNew();
            break;
        case 2:
            createViewportTop();
            break;
        case 3:
            createViewportPlaylist();
            break;
        case 4:
            createViewportSetting();
            break;
       
            
        default:
            break;
    }
    
    return success;
    
}

_Bool createBottomMenu(void)
{
    _Bool success = 1;
    
    SDL_Rect viewportDeuxSize;
    viewportDeuxSize.h = 80;
    viewportDeuxSize.w = 1152;
    viewportDeuxSize.x = 0;
    viewportDeuxSize.y = 568;
   
    SDL_RenderSetViewport( renderer, &viewportDeuxSize );
    
    loadFromFile(CHEMIN"Ressources/img/background.png", &background);
    renderLTexture(background , -500,0, NULL);
    
    SDL_SetRenderDrawColor( renderer, 108, 108, 108, 0xFF );
    SDL_RenderDrawLine( renderer,0,0,1152,0 );
    

    loadFromFile(CHEMIN"Ressources/img/play.png", &play);
    play.mHeight = 25;
    play.mWidth = 25;
    renderLTexture(play,553,27.5, NULL);
    
    loadFromFile(CHEMIN"Ressources/img/skip_right.png", &nextRight);
    nextRight.mHeight = 25;
    nextRight.mWidth = 25;
    renderLTexture(nextRight,598,27.5, NULL);
    
    loadFromFile(CHEMIN"Ressources/img/skip_left.png", &nextLeft);
    nextLeft.mHeight = 25;
    nextLeft.mWidth = 25;
    renderLTexture(nextLeft,508,27.5, NULL);
    
    loadFromFile(CHEMIN"Ressources/img/shuffle.png", &randomMusic);
    randomMusic.mHeight = 25;
    randomMusic.mWidth = 25;
    renderLTexture(randomMusic,423,27.5, NULL);
    
    loadFromFile(CHEMIN"Ressources/img/repeat.png", &replay);
    replay.mHeight = 25;
    replay.mWidth = 25;
    renderLTexture(replay,683,27.5, NULL);
    
    loadFromFile(CHEMIN"Ressources/img/sound.png", &sound);
    sound.mHeight = 25;
    sound.mWidth = 25;
    renderLTexture(sound,978,27.5, NULL);
    
    
    SDL_RenderDrawLine(renderer, 1008,40, 1108, 40);
    loadFromFile(CHEMIN"Ressources/img/point.png", &point);
    point.mHeight = 16;
    point.mWidth = 16;
    
    //question
    renderLTexture(point, xVolume, 32, NULL);
    
    
    return success;
    
}

_Bool createCenterMenu(void){

    _Bool success = 1;

    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_Rect viewportSize;
    viewportSize.h = 568;
    viewportSize.w = 952;
    viewportSize.x = 200;
    viewportSize.y = 0;
    SDL_RenderSetViewport( renderer, &viewportSize );

    loadFromRenderedText(&textTexture, "Récents");
    renderLTexture(textTexture, 10, 10, NULL);

    /* EN COURS
    const int width_Scrolling = 1100;
    const int height_Scrolling = 568;

    const int width_Affichage = 952;
    const int height_Affichage = 180;

//    // x_scrolling = 10, y_scrolling = 20
//
//    SDL_Rect scrolling;
//    scrolling.h = 150;
//    scrolling.w = 932;
//    scrolling.x = 10;
//    scrolling.y = 40;
//
//    SDL_RenderDrawRect(renderer, &scrolling);
//    loadFromRenderedText(&textTexture, "Test");
//    renderLTexture(textTexture, 10, 40, &scrolling);
//
//    SDL_RenderGetClipRect(renderer, &scrolling);
*/
    SDL_RenderDrawLine(renderer, 0, 200, 1152, 200);

    //todo optimisation
    loadFromRenderedText(&textTexture, "Ajouter");
    renderLTexture(textTexture, 10, 210, NULL);
    loadFromRenderedText(&textTexture, "un");
    renderLTexture(textTexture, 10, 240, NULL);
    loadFromRenderedText(&textTexture, "fichier :");
    renderLTexture(textTexture, 10, 270, NULL);

    loadFromFile(CHEMIN"Ressources/img/mp3.png", &mp3Texture);
    mp3Texture.mHeight = 90;
    mp3Texture.mWidth = 90;
    renderLTexture(mp3Texture, 100, 210, NULL);

    loadFromRenderedText(&textTexture, "Télécharger par un lien YouTube :");
    renderLTexture(textTexture, 210, 210, NULL);

    //implémentation barre de recherche (encore à améliorer)
    SDL_Rect inputText;
    inputText.h = 35;
    inputText.w = 500;
    inputText.x = 210;
    inputText.y = 250;

    SDL_RenderDrawRect(renderer, &inputText);
    loadFromRenderedText(&inputTextTexture, textFromInput);
    renderLTexture(inputTextTexture, 215, 255, NULL);

    SDL_RenderDrawLine(renderer, 0, 310, 1100, 310);

    loadFromRenderedText(&textTexture, "Playlists");
    renderLTexture(textTexture, 10, 320, NULL);

    SDL_RenderDrawLine(renderer, 0, 470, 1100, 470);

    loadFromRenderedText(&textTexture, "Tags");
    renderLTexture(textTexture, 10, 480, NULL);


    return success;

}

void createViewportTop(void)
{
    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_Rect viewportSize;
    viewportSize.h = 568;
    viewportSize.w = 952;
    viewportSize.x = 200;
    viewportSize.y = 0;
    SDL_RenderSetViewport( renderer, &viewportSize );
}

void createViewportNew(void)
{
    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_Rect viewportSize;
    viewportSize.h = 568;
    viewportSize.w = 952;
    viewportSize.x = 200;
    viewportSize.y = 0;
    SDL_RenderSetViewport( renderer, &viewportSize );
    
    int xImg = 126;
    int xText = 318;
    int xDuration = 570;
    int xTime = 670;
    int xTag = 383;
    int xTextTag = 400;
    
    renderLTexture(addNewText, 300, 22, NULL);
    
    //first
    renderLTexture(voidImg, xImg, 80, NULL);
    renderLTexture(musicTitle, xText, 94, NULL);
    renderLTexture(musicArtiste, xText, 124, NULL);
    renderLTexture(musicDuration, xDuration, 135, NULL);
    renderLTexture(timeAdd, xTime, 135, NULL);
    renderLTexture(textTags, xText, 174, NULL);
    renderLTexture(imgTag, xTag, 166, NULL);
    renderLTexture(nameTag, xTextTag, 174, NULL);
    
    //second
    renderLTexture(voidImg, xImg, 239, NULL);
    renderLTexture(musicTitle, xText, 253, NULL);
    renderLTexture(musicArtiste, xText, 283, NULL);
    renderLTexture(musicDuration, xDuration, 294, NULL);
    renderLTexture(timeAdd, xTime, 294, NULL);
    renderLTexture(textTags, xText, 332, NULL);
    renderLTexture(imgTag, xTag, 324, NULL);
    renderLTexture(nameTag, xTextTag, 332, NULL);
    
    //third
    renderLTexture(voidImg, xImg, 398, NULL);
    renderLTexture(musicTitle, xText, 412, NULL);
    renderLTexture(musicArtiste, xText, 442, NULL);
    renderLTexture(musicDuration, xDuration, 453, NULL);
    renderLTexture(timeAdd, xTime, 453, NULL);
    renderLTexture(textTags, xText, 490, NULL);
    renderLTexture(imgTag, xTag, 483, NULL);
    renderLTexture(nameTag, xTextTag, 490, NULL);
    
    
    
    
}

void createViewportPlaylist(void)
{
    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_Rect viewportSize;
    viewportSize.h = 568;
    viewportSize.w = 952;
    viewportSize.x = 200;
    viewportSize.y = 0;
    SDL_RenderSetViewport( renderer, &viewportSize );
}

void createViewportSetting()
{
    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_Rect viewportSize;
    viewportSize.h = 568;
    viewportSize.w = 952;
    viewportSize.x = 200;
    viewportSize.y = 0;
    SDL_RenderSetViewport( renderer, &viewportSize );
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
    if(!loadFromFile(CHEMIN"Ressources/img/selected.png", &recSelected)){
        printf("Failed to load" CHEMIN "Ressources/img/selected.png \n");
        
    }
    
    renderLTexture(recSelected, X_BUTTON_LEFT_MENU,y, size);
    
}

void setXVolume(int x){
    xVolume = x;
}
