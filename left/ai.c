#include "../player.h"

#ifndef PLAYER_NAME
#error PLAYER_NAME not defined
#endif

static struct {
	player_side_e side;
	object_size_t ball_size;
	object_size_t paddle_size;

	position_t prev_ball_pos;
	position_t prev_paddle_pos;
} player;

static void init(player_side_e side, object_size_t *ball_size, object_size_t *paddle_size)
{
	player.side = side;
	player.ball_size = *ball_size;
	player.paddle_size = *paddle_size;
}

static void move(position_t *ball_pos, position_t *paddle_pos, move_e *move)
{
	player.prev_ball_pos = *ball_pos;
	player.prev_paddle_pos = *paddle_pos;
	*move = MOVE_NONE;
}

player_api_t PLAYER_NAME = {
	.init = init,
	.move = move,
};
