//
//  inputText.h
//  Cmusic
//
//  Created by Arthur dubois on 16/03/2022.
//

#ifndef inputText_h
#define inputText_h
#include "common.h"
#include "initMedia.h"


void startInput(void);
void handleInputEvent(SDL_Event*, char*);
void stopInput(void);

#endif /* inputText_h */
