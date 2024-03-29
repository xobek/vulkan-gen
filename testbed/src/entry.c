#include "game.h"
#include <entry.h>

#include <core/vmemory.h>

b8 create_game(game* game) {
    game->config.x = 500;
    game->config.y = 500;
    game->config.width = 1280;
    game->config.height = 720;
    game->config.name = "vGo Engine";

    game->initialize = game_initialize;
    game->update = game_update;
    game->render = game_render;
    game->on_resize = game_on_resize;

    game->state = vallocate(sizeof(game_state), MEMORY_TAG_GAME);
    game->application_state = 0;
    return true;
}