#include "common.h"
#include "initMedia.h"
#include "render.h"
#include "viewport.h"
#include "mouseButton.h"

void closee(void);

int main(int argc, const char * argv[]) {
    if( !init() )
    {
      printf( "Failed to initialize!\n" );
    }
    else
    {
        //Load media
        if( !loadMediaText()  )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
            //Main loop flag
            _Bool quit = 0;

            //Event handler
            SDL_Event e;
            
            setPositionButtonLeftMenu();
            setPositionButtonBottomMenu();
            
            while( !quit )
            {
                //Handle events on queue
                while( SDL_PollEvent( &e ) != 0 )
                {
                    //User requests quit
                    if( e.type == SDL_QUIT )
                    {
                        quit = 1;
                    }
                    
                    initButtonMenu(&e);
                }
                clearRenderer();
                //createBottomMenu();
                createLeftMenu();
                createCenterMenu();
                createBottomMenu();
               
                SDL_RenderPresent(renderer);
            }
        }
    }
    closee();
    
    return 0;
}

void closee(void)
{
    
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
    freeLtexture(&textTexture);
    freeLtexture(&mp3Texture);
    freeLtexture(&background);
    freeLtexture(&play);
    freeLtexture(&stop);
    freeLtexture(&nextRight);
    freeLtexture(&nextLeft);
    freeLtexture(&randomMusic);
    freeLtexture(&replay);
    freeLtexture(&sound);
    
    


    
    TTF_CloseFont( gFont );
    gFont = NULL;

    //Destroy window
    SDL_DestroyRenderer( renderer );
    renderer = NULL;


    //Destroy window
    SDL_DestroyWindow( window );
    window = NULL;
    


    //Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}
