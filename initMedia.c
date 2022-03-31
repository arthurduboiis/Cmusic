//
//  initMedia.c
//  Cmusic
//
//  Created by Arthur dubois on 29/11/2021.
//

#include <string.h>
#include "initMedia.h"

_Bool inputTag = 0;
_Bool inputPlaylist = 0;
_Bool inputHome = 0;

static void vmsg(const char *prefix, const char *sep, const char *suffix,
                 const char *format, va_list ap)
{
    if (format) {
        char format2[strlen(prefix) + strlen(sep) + strlen(format) +
                     strlen(suffix) + 2];
        sprintf(format2, "%s%s%s%s\n", prefix, sep, format, suffix);
        vfprintf(stderr, format2, ap);
    } else {
        fprintf(stderr, "%s%s\n", prefix, suffix);
    }
}

void msg(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vmsg("", "", "", format, ap);
    va_end(ap);
}

static void verr(const char *format, va_list ap)
{
    int saved_errno = errno;
    vmsg("ERREUR", ": ", ".", format, ap);
    if (saved_errno != 0)
        fprintf(stderr,
                "(errno = %d: %s)\n", saved_errno, strerror(saved_errno));
}

void die(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    verr(format, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}


#define die_if(expr, ...) if (expr) die(__VA_ARGS__); else (void)0

_Bool init(void)
{
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0 )
    {
        fprintf( stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return 0;
    }

    IMG_Init(IMG_INIT_JPG);
    //Create window
    window = SDL_CreateWindow( "CMusic", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( window == NULL )
    {
        fprintf( stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        return 0;
    }
    displayIcon();
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if( renderer == NULL )
    {
        fprintf( stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return 0;
    }

    //Initialize PNG loading
    int imgFlags = IMG_INIT_PNG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) )
    {
        fprintf( stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
        return 0;
    }
    //Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        fprintf( stderr, "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        return 0;
    }

    return 1;
}



_Bool loadMediaText(void)
{
    //Loading success flag
    _Bool success = 1;

    //Open the font coco-sharp
    gFont = TTF_OpenFont( CHEMIN"Ressources/font/arial.ttf", 20 );
    if( gFont == NULL )
    {
        fprintf( stderr, "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = 0;
    }

    return success;
}


#if defined(SDL_TTF_MAJOR_VERSION)
_Bool loadFromRenderedText(LTexture *ltexture, char textureText[])
{
    //Get rid of preexisting texture
    freeLtexture(ltexture);
    SDL_Color colorText = {0,0,0};
    //Render text surface
    SDL_Surface* textSurface;
//    fprintf(stdout , "taille textureText : %d ", strlen(textureText));
//    if(strlen(textureText) > 0 ){
//        textSurface = TTF_RenderUTF8_Blended(gFont, textureText, colorText );
//    }else{
//        textSurface = TTF_RenderUTF8_Blended(gFont, " ", colorText );
//    }
    textSurface = TTF_RenderUTF8_Blended(gFont, textureText, colorText );
    if( textSurface == NULL)
    {
        fprintf(stderr, "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        ltexture->mTexture = SDL_CreateTextureFromSurface( renderer, textSurface );
        if( ltexture->mTexture == NULL )
        {
            fprintf(stderr, "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            ltexture->mWidth = textSurface->w;
            ltexture->mHeight = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }
    
    //Return success
    return ltexture->mTexture != NULL;
}
#endif

_Bool loadFromFile(char path[],LTexture *finalTexture){
    //Get rid of preexisting texture
    freeLtexture(finalTexture);
    //The final texture
    SDL_Texture* newTexture = NULL;
    
    

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path );
    if( loadedSurface == NULL )
    {
        fprintf(stderr, "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
    }
    else
    {
        //Color key image
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format,89, 106, 255 ) );
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
        if( newTexture == NULL )
        {
            fprintf(stderr, "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            finalTexture->mWidth = loadedSurface->w;
            finalTexture->mHeight = loadedSurface->h;
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    //Return success
    finalTexture->mTexture = newTexture;
    return finalTexture->mTexture != NULL;
}

void displayIcon(void){
    SDL_Surface *loadIcon = IMG_Load(CHEMIN"Ressources/img/logo_cmusic.png");
    if (loadIcon == NULL){
        fprintf(stderr, "Unable to load image app icon! SDL_image Error: %s\n", IMG_GetError() );
    } else {
        SDL_SetColorKey( loadIcon, SDL_TRUE, SDL_MapRGB( loadIcon->format,89, 106, 255 ));
        SDL_SetWindowIcon(window, loadIcon);
    }
}

void freeLtexture(LTexture *texture)
{
    //Free texture if it exists
    if( texture->mTexture != NULL )
    {
        SDL_DestroyTexture( texture->mTexture );
        texture->mTexture = NULL;
        texture->mWidth = 0;
        texture->mHeight = 0;
    }
}

void renderLTexture(LTexture texture,  int x, int y, SDL_Rect *clip){
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y, texture.mWidth, texture.mHeight };
   
    //Set clip rendering dimensions
    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    
    //Render to screen
    SDL_RenderCopy( renderer, texture.mTexture, clip, &renderQuad );
}

SDL_Texture* loadMediaTexture(char path[])
{
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}

void initAllLTexture(void){
    
    // init left menu
    die_if(!loadFromFile(CHEMIN"Ressources/img/logo_cmusic.png", &logoTexture), "Can't load logo");
    die_if(!loadFromFile(CHEMIN"Ressources/img/selected.png", &recSelected), "Can't load rectSelected");
    die_if(!loadFromRenderedText(&homeTxt, "Accueil"),"Can't load text : Acceuil");
    die_if(!loadFromFile(CHEMIN"Ressources/img/home.png", &homePng), "Can't load img home");
    die_if(!loadFromRenderedText(&newTxt, "Nouveautés"), "Can't load text : Nouveautés");
    die_if(!loadFromFile(CHEMIN"Ressources/img/new.png", &newPng), "Can't load img new");
    die_if(!loadFromRenderedText(&topTxt, "Top"), "Can't load text : Top");
    die_if(!loadFromFile(CHEMIN"Ressources/img/rank.png", &topPng), "Can't load img top");
    die_if(!loadFromRenderedText(&playlistTxt, "Playlists"), "Can't load text : playlist");
    die_if(!loadFromFile(CHEMIN"Ressources/img/playlist.png", &playlistPng), "Can't load img playlist");
    die_if(!loadFromRenderedText(&paramTxt, "Paramètres"), "Can't load text : Paramètre");
    die_if(!loadFromFile(CHEMIN"Ressources/img/setting.png", &paramPng), "Can't load img setting");
    
    
    // init bottom menu
    die_if(!loadFromFile(CHEMIN"Ressources/img/background.png", &background), "Can't load background img for bottom menu");
    die_if(!loadFromFile(CHEMIN"Ressources/img/play.png", &play), "Can't load Play img ");
    die_if(!loadFromFile(CHEMIN"Ressources/img/pause.png", &pauseB), "Can't load Play img ");
    die_if(!loadFromFile(CHEMIN"Ressources/img/skip_right.png", &nextRight), "Can't load skip right img");
    die_if(!loadFromFile(CHEMIN"Ressources/img/skip_left.png", &nextLeft), "Can't load skip left img");
    die_if(!loadFromFile(CHEMIN"Ressources/img/skip_rightB.png", &nextRightB), "Can't load skip right img");
    die_if(!loadFromFile(CHEMIN"Ressources/img/skip_leftB.png", &nextLeftB), "Can't load skip left img");
    die_if(!loadFromFile(CHEMIN"Ressources/img/shuffle.png", &randomMusic), "Can't load shuffle img");
    die_if(!loadFromFile(CHEMIN"Ressources/img/shuffleB.png", &randomMusicB), "Can't load shuffle img");
    die_if(!loadFromFile(CHEMIN"Ressources/img/repeat.png", &replay), "Can't load repeat img ");
    die_if(!loadFromFile(CHEMIN"Ressources/img/repeatB.png", &replayB), "Can't load repeat img ");
    die_if(!loadFromFile(CHEMIN"Ressources/img/sound.png", &sound), "Can't load sound img");
    die_if(!loadFromFile(CHEMIN"Ressources/img/point.png", &point),"Can't load point img");
    
    // init viewport news
    die_if(!loadFromRenderedText(&addNewText, "Ajouté récemment"), "Can't load text : Ajouté récemment");
    die_if(!loadFromFile(CHEMIN"Ressources/img/void.png", &voidImg), "Can't load void img");
    die_if(!loadFromRenderedText(&musicTitle, "Titre de la musique"), "Can't load text : Titre de la musique");
    die_if(!loadFromRenderedText(&musicArtiste, "Artiste"), "Can't load text : Artiste");
    die_if(!loadFromRenderedText(&musicDuration, "04:34"), "Can't load text : Music duration");
    die_if(!loadFromRenderedText(&textPlaylist, "Playlist : "), "Can't load text : Playlist : ");
    die_if(!loadFromRenderedText(&textTags, "Tags : "), "Can't load text : tags :");
    die_if(!loadFromRenderedText(&nameTag, "Hip-Hop"), "Can't load text : nameTag");
    die_if(!loadFromFile(CHEMIN"Ressources/img/imgTag.png", &imgTag), "Can't load imgTag");
    
    // init viewport top
    die_if(!loadFromRenderedText(&listenText, "Les plus écoutés"), "Can't load text : Les plus écoutés");
    die_if(!loadFromRenderedText(&hashtag, "#"), " Can't load text : #");
    die_if(!loadFromRenderedText(&title, "Titre"), "Can't load text : Title");
    die_if(!loadFromRenderedText(&artist, "Artiste"), "Can't load text : Artiste");
    die_if(!loadFromRenderedText(&duration, "Durée"), "Can't load text : Durée");
    die_if(!loadFromRenderedText(&views, "Vues"), "Can't load text : Vues");
    
    
   
    
    
    
}

void freeAllTexture(void){

    // free text input
    freeLtexture(&inputTextTexture);

    // free center menu
    freeLtexture(&textTexture);
    freeLtexture(&mp3Texture);
    freeLtexture(&mp3BTexture);

    // free left menu
    freeLtexture(&recSelected);
    freeLtexture(&logoTexture);
    freeLtexture(&homeTxt);
    freeLtexture(&homePng);
    freeLtexture(&newTxt);
    freeLtexture(&newPng);
    freeLtexture(&topTxt);
    freeLtexture(&topPng);
    freeLtexture(&playlistTxt);
    freeLtexture(&playlistPng);
    freeLtexture(&paramTxt);
    freeLtexture(&paramPng);
    
    
    //free bottom menu
    freeLtexture(&background);
    freeLtexture(&play);
    freeLtexture(&pauseB);
    freeLtexture(&stop);
    freeLtexture(&nextRight);
    freeLtexture(&nextLeft);
    freeLtexture(&nextRightB);
    freeLtexture(&nextLeftB);
    freeLtexture(&randomMusic);
    freeLtexture(&randomMusicB);
    freeLtexture(&replay);
    freeLtexture(&replayB);
    freeLtexture(&sound);
    freeLtexture(&point);
    
    
    
    // free viewport news
    freeLtexture(&addNewText);
    freeLtexture(&voidImg);
    freeLtexture(&musicTitle);
    freeLtexture(&musicArtiste);
    freeLtexture(&musicDuration);
    freeLtexture(&textTags);
    freeLtexture(&nameTag);
    freeLtexture(&imgTag);
    freeLtexture(&textPlaylist);
    freeLtexture(&textVarTag);
    // free viewport top
    freeLtexture(&listenText);
    freeLtexture(&hashtag);
    freeLtexture(&title);
    freeLtexture(&artist);
    freeLtexture(&duration);
    freeLtexture(&views);
    
    
    
}

int getWidth(LTexture *texture){
    return texture->mWidth;
}
int getHeight(LTexture *texture){
    return texture->mHeight;
}

void setActiveInputTag(_Bool active)
{
    inputTag = active;
}

void setActiveInputPlaylist(_Bool active)
{
    inputPlaylist = active;
}

void setActiveInputHome(_Bool active)
{
    inputHome = active;
}

_Bool getInputTag(void){
    return inputTag;
}

_Bool getInputPlaylist(void){
    return inputPlaylist;
}

_Bool getInputHome(void){
    return inputHome;
}

