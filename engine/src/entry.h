#pragma once 

#include "core/application.h"
#include "core/logger.h"
#include "gametypes.h"

extern b8 create_game(game* out_game);

int main(void) {

    game game_inst;
    if (!create_game(&game_inst)) {
        ERROR("Failed to create game instance!");
        return -1;
    }

    if (!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.on_resize) {
        ERROR("Game instance is missing required function pointers!");
        return -2;
    }


    // Initalize game
    if(!application_create(&game_inst)) {
        ERROR("Failed to create application!");
        return 1;
    }
    
    // Start of game loop
    if(!application_run()) {
        ERROR("Failed to run application!");
        return 2;
    }

    return 0;
}