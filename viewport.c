//
//  viewport.c
//  Cmusic
//
//  Created by Arthur dubois on 02/12/2021.
//

#include "viewport.h"

int selectedButton = 0;
int xVolume=1098;
int yImgOfMusic = 80;
int yTitleOfMusic = 94;
int yTitleOfArtiste = 124;
int yMusicDuration = 135;
int yAddButton = 174;
int yAddPlaylist = 224;
int indexAddButton = 0;


int xScrolling = 50;

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
    
    
    renderLTexture(logoTexture, 50, 15, NULL);
    
    recSelectedSize.h = BUTTON_HEIGHT;
    recSelectedSize.w = BUTTON_WIDTH;
    recSelectedSize.x = 0;
    recSelectedSize.y = 0;
    

    buttonSelected();
    setRectSelected(&recSelectedSize, gButtonsLeftMenu[selectedButton].mPosition.y);
    
    
    renderLTexture(homeTxt, 62, 207, NULL);
    homePng.mHeight = 25;
    homePng.mWidth = 25;
    renderLTexture(homePng, 20,202, NULL);
    
    
    renderLTexture(newTxt, 62, 257, NULL);
    newPng.mHeight = 25;
    newPng.mWidth = 25;
    renderLTexture(newPng, 20, 256, NULL);
    
    
    renderLTexture(topTxt, 62, 307, NULL);
    topPng.mHeight = 25;
    topPng.mWidth = 25;
    renderLTexture(topPng, 20, 306, NULL);
    

    renderLTexture(playlistTxt, 62, 357, NULL);
    playlistPng.mHeight = 25;
    playlistPng.mWidth = 25;
    renderLTexture(playlistPng, 20, 356, NULL);
    
    
    renderLTexture(paramTxt, 62, 532, NULL);
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
    
    renderLTexture(background , -500,0, NULL);
    
    SDL_SetRenderDrawColor( renderer, 108, 108, 108, 0xFF );
    SDL_RenderDrawLine( renderer,0,0,1152,0 );
    

    
    play.mHeight = 25;
    play.mWidth = 25;
    renderLTexture(play,553,27.5, NULL);
    
    nextRight.mHeight = 25;
    nextRight.mWidth = 25;
    renderLTexture(nextRight,598,27.5, NULL);
    
    
    nextLeft.mHeight = 25;
    nextLeft.mWidth = 25;
    renderLTexture(nextLeft,508,27.5, NULL);
    
    randomMusic.mHeight = 25;
    randomMusic.mWidth = 25;
    renderLTexture(randomMusic,423,27.5, NULL);
    

    replay.mHeight = 25;
    replay.mWidth = 25;
    renderLTexture(replay,683,27.5, NULL);
    
    sound.mHeight = 25;
    sound.mWidth = 25;
    renderLTexture(sound,978,27.5, NULL);
    
    
    SDL_RenderDrawLine(renderer, 1008,40, 1136, 40);
    point.mHeight = 16;
    point.mWidth = 16;
    
    //question
    renderLTexture(point, xVolume, 32, NULL);
    
    
    return success;
    
}

// Définition des éléments graphiques de la partie centrale
_Bool createCenterMenu(void){

    _Bool success = 1;

    // Viewport affichant les éléments
    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_Rect viewportSize;
    viewportSize.h = 40;
    viewportSize.w = 952;
    viewportSize.x = 200;
    viewportSize.y = 0;
    SDL_RenderSetViewport( renderer, &viewportSize );

    loadFromRenderedText(&textTexture, "Récents");
    renderLTexture(textTexture, 10, 10, NULL);

    // Nouveau viewport définie pour le scrolling
    SDL_Rect scrolling;
    scrolling.h = 150;
    scrolling.w = 932;
    scrolling.x = 210;
    scrolling.y = 40;

    SDL_RenderSetViewport(renderer, &scrolling);

    scrolling.x = 0;
    scrolling.y = 0;
    SDL_RenderDrawRect(renderer, &scrolling);

    spacing = 0;
    for (int i = 0; i < TOTAL_BUTTON_SCROLLING_AREA; ++i) {
        loadFromFile(CHEMIN"Ressources/img/mp3.png", &mp3Texture);
        mp3Texture.mHeight = 100;
        mp3Texture.mWidth = 100;
        renderLTexture(mp3Texture, xScrolling + spacing, 10, NULL);
        char test[100];
        sprintf(test, "Test %d", i);
        loadFromRenderedText(&textTexture, test);
        renderLTexture(textTexture, xScrolling + spacing + 25, 120, NULL);

        spacing+=250;
    }

    viewportSize.h = 378;
    viewportSize.y = 190;
    SDL_RenderSetViewport(renderer, &viewportSize);

    SDL_RenderDrawLine(renderer, 0, 10, 1152, 10);

    loadFromRenderedText(&textTexture, "Ajouter");
    renderLTexture(textTexture, 10, 20, NULL);
    loadFromRenderedText(&textTexture, "un");
    renderLTexture(textTexture, 10, 50, NULL);
    loadFromRenderedText(&textTexture, "fichier :");
    renderLTexture(textTexture, 10, 80, NULL);

    loadFromFile(CHEMIN"Ressources/img/mp3.png", &mp3Texture);
    mp3Texture.mHeight = 90;
    mp3Texture.mWidth = 90;
    renderLTexture(mp3Texture, 100, 20, NULL);
    setDropButton(90, 90, 300, 20);
    

    loadFromRenderedText(&textTexture, "Télécharger par un lien YouTube :");
    renderLTexture(textTexture, 210, 20, NULL);


    SDL_Rect inputText;
    inputText.h = 35;
    inputText.w = 500;
    inputText.x = 210;
    inputText.y = 60;

    SDL_RenderDrawRect(renderer, &inputText);
//    size_t m = sizeof(textFromInput)/sizeof(textFromInput[0])
    loadFromRenderedText(&inputTextTexture, textFromInput);
    renderLTexture(inputTextTexture, 215, 65, NULL);

    SDL_RenderDrawLine(renderer, 0, 120, 1100, 120);

    loadFromRenderedText(&textTexture, "Playlists");
    renderLTexture(textTexture, 10, 130, NULL);

    SDL_RenderDrawLine(renderer, 0, 280, 1100, 280);

    loadFromRenderedText(&textTexture, "Tags");
    renderLTexture(textTexture, 10, 290, NULL);


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
    
    int yTitle = 127;
    
    renderLTexture(listenText, 330, 9, NULL);
    SDL_RenderDrawLine(renderer, 92, 111, 862, 111);
    
    renderLTexture(hashtag, 102, yTitle, NULL);
    renderLTexture(title, 180, yTitle, NULL);
    renderLTexture(artist, 425, yTitle, NULL);
    renderLTexture(duration, 606, yTitle, NULL);
    renderLTexture(views, 737, yTitle, NULL);
    
    for(int i = 0; i < 8 ; i++ ){
        SDL_RenderDrawLine(renderer, 92, 166 + (i*50), 862, 166 + (i * 50));
    }
    
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
    

    
    renderLTexture(addNewText, 300, 22, NULL);
    
    createMusicDisplay(voidImg,musicTitle,musicArtiste,musicDuration);

    /*//first
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
    renderLTexture(nameTag, xTextTag, 490, NULL);*/
    
    
    
    
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

void createMusicDisplay(LTexture img,LTexture title, LTexture artiste, LTexture musicDuration){


    int xImg = 126;
    int xText = 318;
    int xDuration = 570;


    //first
    renderLTexture(img, xImg, yImgOfMusic, NULL);
    renderLTexture(title, xText, yTitleOfMusic, NULL);
    renderLTexture(artiste, xText, yTitleOfArtiste, NULL);
    renderLTexture(musicDuration, xDuration, yMusicDuration, NULL);

    renderLTexture(textTags, xText, yAddButton, NULL);
    setPositionButtonViewportNew();



    //TODO :
    // si chaine != vide et pas en train d'écrire :
    // afficher le tag

    renderLTexture(textPlaylist, xText, yAddPlaylist, NULL);

    inputText.w = 200;
    inputText.x = xText+90;
    inputText.y = yAddButton-5;

    SDL_RenderDrawRect(renderer, &inputText);
    if(textAddTagInput[0] != '\0'){
        startInput();
        loadFromRenderedText(&inputTextTexture, textAddTagInput);
    }else {
        loadFromRenderedText(&inputTextTexture, " ");
    }



    renderLTexture(inputTextTexture, inputText.x, inputText.y, NULL);

    if(wantInputButtons[0].mCurrentButton == 3) {
        setActiveInputTag(1);
        setActiveInputPlaylist(0);
    }

    if(textAddTagInput[0] != '\0' && !getInputTag()){
        renderLTexture(imgTag, xText+310, yAddButton-10, NULL);
        loadFromRenderedText(&textVarTag, textAddTagInput);
        renderLTexture(textVarTag, xText+320, yAddButton, NULL);
    }

    //renderLTexture(imgTag, xText+400, yAddButton-10, NULL);




    inputText.y = yAddButton + 40;

    SDL_RenderDrawRect(renderer, &inputText);

    if(textAddPlaylistInput[0] != '\0'){
        startInput();
        loadFromRenderedText(&inputTextTexture, textAddPlaylistInput);
    }else {
        loadFromRenderedText(&inputTextTexture, " ");
    }

    renderLTexture(inputTextTexture, inputText.x, inputText.y, NULL);

    if(wantInputButtons[1].mCurrentButton == 3){
        setActiveInputPlaylist(1);
        setActiveInputTag(0);
    }

    indexAddButton++;
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

void setXScrolling(int x){
    xScrolling = x;
}

int getSelectedPage(void){
    return selectedButton;
}
