//
//  common.h
//  Cmusic
//
//  Created by Arthur dubois on 29/11/2021.
//

#ifndef common_h
#define common_h

#ifdef _WIN32

    #include <stdio.h>
    #include <string.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_ttf.h>
    #include <SDL2/SDL_thread.h>

#define CHEMIN "../"

#endif

#ifdef __linux__
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_ttf.h>
    #include <SDL2/SDL_thread.h>
#define CHEMIN "../"
#endif

#ifdef __APPLE__
    #include <stdio.h>
    #include <SDL2_image/SDL_image.h>
    #include <SDL2/SDL.h>
    #include <SDL2_ttf/SDL_ttf.h>

    #define CHEMIN "./"
#endif


#include <unistd.h>
#include <errno.h>
#endif /* common_h */
