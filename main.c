#include <string.h>
#include "common.h"
#include "initMedia.h"
#include "render.h"
#include "viewport.h"
#include "mouseButton.h"
#include "dropEvent.h"
#include "inputText.h"


void closee(void);

int main(int argc, char* argv[]){
    if(!init()){
        fprintf(stderr, "Failed to initialize\n");
        closee();
        return -1;
    }
    if(!loadMediaText()){
        fprintf(stderr, "Failed to load media\n");
        closee();
        return -1;
    }

    //SDL_CreateThread(thread_lecteur_audio, "thread_lecteur_audio", "rtmp://localhost/live/STREAM_NAME");

    initAllLTexture();
    //Main loop flag
    _Bool quit = 0;
    //Event handler
    SDL_Event e;

    
    textFromInput = malloc(100 * sizeof(char*));
    textAddTagInput = malloc(100 * sizeof(char*));
    textAddPlaylistInput = malloc(100 * sizeof(char*));
    
    setPositionButtonLeftMenu();
    setPositionButtonBottomMenu();
    

    
   
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    while( !quit )
    {

        //_Bool renderText = 0;
        clearRenderer();
        
        createLeftMenu();
        createBottomMenu();

        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = 1;
            }

            
            switch (getSelectedPage()) {
                case 0:
                    
                    handleInputEvent(&e, textFromInput);
                    break;
                case 1:
                    if(getInputTag())handleInputEvent(&e, textAddTagInput);
                    if(getInputPlaylist())handleInputEvent(&e, textAddPlaylistInput);
                default:
                    break;
            }
            
            dropEvent(&e);

            initButtonMenu(&e);
            dragButtonVolume(&e);
        }




        SDL_RenderPresent(renderer);
    }

    closee();

    return 0;
}

void closee(void)
{
    freeAllTexture();


    TTF_CloseFont( gFont );
    gFont = NULL;

    //Destroy window
    SDL_DestroyRenderer( renderer );
    renderer = NULL;


    //Destroy window
    SDL_DestroyWindow( window );
    window = NULL;
    
    free(textFromInput);
    free(textAddTagInput);
    free(textAddPlaylistInput);

    //Quit SDL subsystems
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}
