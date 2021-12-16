#include <string.h>
#include "common.h"
#include "initMedia.h"
#include "render.h"
#include "viewport.h"
#include "mouseButton.h"

void closee(void);

int main(int argc,char * argv[]) {
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

            textFromInput = malloc(100 * sizeof(char*));
            
            

            loadFromRenderedText(&inputTextTexture, textFromInput);

            SDL_StartTextInput();
            
            setPositionButtonLeftMenu();
            setPositionButtonBottomMenu();
            
            
            
            while( !quit )
            {
                
                _Bool renderText = 0;
                clearRenderer();
                createLeftMenu();
                //createCenterMenu();
                createBottomMenu();

                //Handle events on queue
                while( SDL_PollEvent( &e ) != 0 )
                {
                    //User requests quit
                    if( e.type == SDL_QUIT )
                    {
                        quit = 1;
                    }
                    else if( e.type == SDL_KEYDOWN )
                    {
                        //Handle backspace
                        if( e.key.keysym.sym == SDLK_BACKSPACE && strlen(textFromInput) > 0 )
                        {
                            //lop off character
                            textFromInput[strlen(textFromInput)-1] = '\0';
                            renderText = 1;
                        }
                            //Handle copy
                        else if( e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
                        {
                            SDL_SetClipboardText( textFromInput );
                        }
                            //Handle paste
                        else if( e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
                        {
                            textFromInput = SDL_GetClipboardText();
                            renderText = 1;
                        }

                    }

                        //Special text input event
                    else if( e.type == SDL_TEXTINPUT ) {
                        //Not copy or pasting
                        if (!(SDL_GetModState() & KMOD_CTRL &&
                              (e.text.text[0] == 'c' || e.text.text[0] == 'C' || e.text.text[0] == 'v' ||
                               e.text.text[0] == 'V'))) {

                            //inputText += e.text.text;
                            strcat(textFromInput, e.text.text);
                            //inputText = inputText +  e.text.text;
                            renderText = 1;
                        }
                    }
                    initButtonMenu(&e);
                    dragButtonVolume(&e);
                }
                
               
                
               
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
    freeLtexture(&point);
    
    


    
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
