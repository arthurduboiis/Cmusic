//
//  mouseButton.c
//  Cmusic
//
//  Created by Arthur dubois on 08/12/2021.
//

#include "mouseButton.h"
#include "lecteur_audio.h"

int volume = 128;

void initLButton(LButton *button){
    button->mPosition.x = 0;
    button->mPosition.y = 0;

    button->mCurrentButton = BUTTON_MOUSE_OUT;
}

void setPosition(LButton *button, int x, int y )
{
    button->mPosition.x = x;
    button->mPosition.y = y;
}

void handleEvent( LButton *button, SDL_Event* e ,int width, int height)
{
    //If mouse event happened
    if( e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP || e->type == SDL_MOUSEWHEEL)
    {
        //Get mouse position
        int x, y;
        SDL_GetMouseState( &x, &y );
        //Check if mouse is in button
        _Bool inside = 1;

            //Mouse is left of the button
            if (x < button->mPosition.x) {
                inside = 0;
            }
                //Mouse is right of the button
            else if (x > button->mPosition.x + width) {
                inside = 0;
            }
                //Mouse above the button
            else if (y < button->mPosition.y) {
                inside = 0;
            }
                //Mouse below the button
            else if (y > button->mPosition.y + height) {
                inside = 0;
            }
            //Mouse is outside button
            if (!inside) {
                button->mCurrentButton = BUTTON_MOUSE_OUT;
            }
                //Mouse is inside button
            else {
                if (e->type == SDL_MOUSEMOTION) {
                    button->mCurrentButton = BUTTON_MOUSE_MOTION;
                }
                if (e->type == SDL_MOUSEBUTTONDOWN) {
                    button->mCurrentButton = BUTTON_MOUSE_DOWN;
                    printf("downnnnn");

                }

                if (e->type == SDL_MOUSEBUTTONUP) {
                    button->mCurrentButton = BUTTON_MOUSE_UP;
                    printf("clicked");
                }

                if (e->type == SDL_MOUSEWHEEL ) {

                    if (e->wheel.y > 0 ){
                        button->mCurrentButton = WHEEL_UP;
                    }else if (e->wheel.y < 0){
                        button->mCurrentButton = WHEEL_DOWN;
                    }
                }


            }
        }
    }


void initButtonMenu(SDL_Event *e)
{
    for(int i = 0; i< TOTAL_BUTTONS_LEFT_MENU; i++)
    {
        handleEvent(&gButtonsLeftMenu[i], e, BUTTON_WIDTH, BUTTON_HEIGHT);
        handleEvent(&gButtonsBottomMenu[i], e,BUTTON_BOTTOM_WIDTH,BUTTON_BOTTOM_HEIGHT);
    }
    handleEvent(&volumeButton, e, 100, BUTTON_VOLUME_SIZE);
    handleEvent(&scrollingArea, e, SCROLLING_AREA_WIDTH, SCROLLING_AREA_HEIGHT);
    for(int i = 0; i < TOTAL_BUTTON_SCROLLING_AREA; i++)
    {
        handleEvent(&scrollingButton[i], e, SCROLLING_BUTTON_MUSIC_WIDTH, SCROLLING_BUTTON_MUSIC_HEIGHT);
    }
    /*for(int i = 0; i < TOTAL_ADD_BUTTONS; i++)
    {
        handleEvent(&addButtons[i], e, BUTTON_WIDTH, BUTTON_HEIGHT);
    }*/

    handleEvent(&wantInputButtons[0], e, 200, 50);
    handleEvent(&wantInputButtons[1], e, 200, 50);

}

void dragButtonVolume(SDL_Event *event)
{
    if(volumeButton.mCurrentButton == 2)
    {
        setXVolume(event->motion.x - BUTTON_VOLUME_SIZE/2);
        volume = event->motion.x - 1008;

        fprintf(stderr, "volume : %d\n", volume);
    }
}

void scrollingEvent()
{
    if(scrollingArea.mCurrentButton == 4 && xScrolling < 10)
    {
        setXScrolling(xScrolling+= 60);
    }
    else if (scrollingArea.mCurrentButton == 5 && xScrolling > -1930)
    {
        setXScrolling(xScrolling-= 60);
    }
}

void setPositionButtonLeftMenu(void)
{
    setPosition(&gButtonsLeftMenu[0], X_BUTTON_LEFT_MENU,Y_BUTTON_HOME);
    setPosition(&gButtonsLeftMenu[1], X_BUTTON_LEFT_MENU,Y_BUTTON_NEW);
    setPosition(&gButtonsLeftMenu[2], X_BUTTON_LEFT_MENU,Y_BUTTON_RANK);
    setPosition(&gButtonsLeftMenu[3], X_BUTTON_LEFT_MENU,Y_BUTTON_PLAYLIST);
    setPosition(&gButtonsLeftMenu[4], X_BUTTON_LEFT_MENU,Y_BUTTON_SETTING);
    
}

void setPositionButtonBottomMenu(void)
{
    setPosition(&gButtonsBottomMenu[0], X_BUTTON_SHUFFLE,Y_BUTTON_BOTTOM_MENU);
    setPosition(&gButtonsBottomMenu[1], X_BUTTON_SKIP_LEFT,Y_BUTTON_BOTTOM_MENU);
    setPosition(&gButtonsBottomMenu[2], X_BUTTON_PLAY,Y_BUTTON_BOTTOM_MENU);
    setPosition(&gButtonsBottomMenu[3], X_BUTTON_SKIP_RIGHT,Y_BUTTON_BOTTOM_MENU);
    setPosition(&gButtonsBottomMenu[4], X_BUTTON_REPEAT,Y_BUTTON_BOTTOM_MENU);
    setPosition(&volumeButton, 1008, 600);
    
}

void setPositionScrollingArea(void)
{
    setPosition(&scrollingArea, 210, 40);
    for (int i = 0; i < TOTAL_BUTTON_SCROLLING_AREA; ++i) {
        setPosition(&scrollingButton[i], xScrolling, 60);
    }

}

void setPositionButtonViewportNew(void)
{
    setPosition(&wantInputButtons[0], 598, 169);
    setPosition(&wantInputButtons[1], 598, 214);
}


