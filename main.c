#include "common.h"
#include "initMedia.h"
#include "render.h"
#include "viewport.h"



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
                }
                clearRenderer();
                createLeftMenu();
                SDL_RenderPresent(renderer);
            }
        }
    }
    closee();
    
    return 0;
}
