#include "game.h"
#include <entry.h>

#include <platform/platform.h>

b8 create_game(game* game) {
    game->config.x = 100;
    game->config.y = 100;
    game->config.width = 800;
    game->config.height = 600;
    game->config.name = "vGo Engine";

    game->initialize = game_initialize;
    game->update = game_update;
    game->render = game_render;
    game->on_resize = game_on_resize;

    game->state = platform_allocate(sizeof(game_state), 0);
    return TRUE;
}