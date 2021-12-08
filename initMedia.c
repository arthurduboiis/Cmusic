//
//  initMedia.c
//  Cmusic
//
//  Created by Arthur dubois on 29/11/2021.
//

#include "initMedia.h"


_Bool init(void)
{
    //Initialization flag
    _Bool success = 1;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = 0;
    }
    else
    {
        IMG_Init(IMG_INIT_JPG);
        //Create window
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = 0;
        }
        else
        { //Create renderer for window<
            renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
            if( renderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = 0;
            }else {
                //Initialize renderer color
                //SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                
                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = 0;
                }
                //Initialize SDL_ttf
               if( TTF_Init() == -1 )
               {
                   printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                   success = 0;
               }
            }
           
        }
    }

    return success;
}



_Bool loadMediaText(void)
{
    //Loading success flag
    _Bool success = 1;
    
    

    //Open the font
    gFont = TTF_OpenFont( "./Ressources/font/coco-sharp.ttf", 20 );
    if( gFont == NULL )
    {
        printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
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
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(gFont, textureText, colorText );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        ltexture->mTexture = SDL_CreateTextureFromSurface( renderer, textSurface );
        if( ltexture->mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
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
        printf( "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
    }
    else
    {
        //Color key image
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format,89, 106, 255 ) );
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
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



int getWidth(LTexture *texture){
    return texture->mWidth;
}
int getHeight(LTexture *texture){
    return texture->mHeight;
}


        

