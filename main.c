#include <string.h>
#include "common.h"
#include "initMedia.h"
#include "render.h"
#include "viewport.h"
#include "mouseButton.h"
#include "dropEvent.h"
#include "inputText.h"
#include "lecteur_audio.h"
#include "tcp.h"
#include "flag_thread.h"

_Bool quit = 0;

void closee(void);

int main(int argc, char* argv[]){

//    freopen("error_log.txt", "w", stderr);

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

    char temp[] = "127.0.0.1:8080";

    SDL_CreateThread(thread_tcp, "thread_tcp", temp);

    initAllLTexture();
    //Main loop flag
    //Event handler
    SDL_Event e;


    textFromInput = "Ajouter une musique";
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
            setPositionScrollingArea();

            initButtonMenu(&e);
            dragButtonVolume(&e);
            checkRR();
            chooseButton();
            scrollingEvent();
            playButton();
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
//    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}
