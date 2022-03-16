//
//  inputText.c
//  Cmusic
//
//  Created by Arthur dubois on 16/03/2022.
//

#include "inputText.h"

char* startInput(char *textFromInput, LTexture inputTextTexture){
    textFromInput = malloc(100 * sizeof(char*));

    

    SDL_StartTextInput();
    
    return textFromInput;
}

void handleInputEvent(SDL_Event *e, char *textFromInput)
{
    if( e->type == SDL_KEYDOWN )
    {
        //Handle backspace
        if( e->key.keysym.sym == SDLK_BACKSPACE && strlen(textFromInput) > 0 )
        {
            //lop off character
            textFromInput[strlen(textFromInput)-1] = '\0';
            
        }
            //Handle copy
        else if( e->key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
        {
            SDL_SetClipboardText( textFromInput );
        }
            //Handle paste
        else if( e->key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
        {
            textFromInput = SDL_GetClipboardText();
            
        }
        
        // Press enter to end input text
        else if(e->key.keysym.sym == SDLK_RETURN)
        {
            SDL_StopTextInput();
            printf("%s", textFromInput);
        }
    }
        //Special text input event
    else if( e->type == SDL_TEXTINPUT ) {
        //Not copy or pasting
        if (!(SDL_GetModState() & KMOD_CTRL &&
              (e->text.text[0] == 'c' || e->text.text[0] == 'C' || e->text.text[0] == 'v' ||
               e->text.text[0] == 'V'))) {

            //inputText += e.text.text;
            strcat(textFromInput, e->text.text);
            //inputText = inputText +  e.text.text;
            
        }
    }
}
