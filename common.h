//
//  common.h
//  Cmusic
//
//  Created by Arthur dubois on 29/11/2021.
//

#ifndef common_h
#define common_h

#define WINDOWS

#ifdef WINDOWS

#include <stdio.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#define CHEMIN "../"

#endif

#ifdef LINUX
/ Code source pour Linux /
#endif

#ifdef MAC
    #include <stdio.h>
    #include <SDL2_image/SDL_image.h>
    #include <SDL2/SDL.h>
    #include <SDL2_mixer/SDL_mixer.h>
    #include <SDL2_ttf/SDL_ttf.h>

    #define CHEMIN "./"
#endif





#include <unistd.h>
#include <errno.h>
#endif /* common_h */
