cmake_minimum_required(VERSION 3.20)
project(CMusic)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(CMusic
        Ressources/font/arial.ttf
        Ressources/font/coco-sharp.ttf
        Ressources/img/background.png
        Ressources/img/home.png
        Ressources/img/logo_cmusic.png
        Ressources/img/mp3.png
        Ressources/img/new.png
        Ressources/img/pause.png
        Ressources/img/play.png
        Ressources/img/playlist.png
        Ressources/img/rank.png
        Ressources/img/repeat.png
        Ressources/img/selected.png
        Ressources/img/setting.png
        Ressources/img/shuffle.png
        Ressources/img/skip_left.png
        Ressources/img/skip_right.png
        Ressources/img/sound.png
        common.h
        initMedia.c
        initMedia.h
        main.c
        mouseButton.c
        mouseButton.h
        render.c
        render.h
        viewport.c
        viewport.h
        dropEvent.c
        dropEvent.h
        inputText.c
        inputText.h)

target_link_libraries(CMusic mingw32 sdl2main sdl2 sdl2_image sdl2_ttf sdl2_mixer)
