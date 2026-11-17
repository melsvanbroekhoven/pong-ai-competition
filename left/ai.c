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

static void move(position_t *ball_pos, position_t *paddle_pos, move_t *move)
{
	int ball_dy = player.prev_ball_pos.y - ball_pos->y;
	int ball_dx = player.prev_ball_pos.x - ball_pos->x;

	int paddle_mid_y = paddle_pos->y + player.paddle_size.height/2;
	*move = ball_pos->y - paddle_mid_y;
	if (*move < MOVE_MIN) {
		*move = MOVE_MIN;
	} else if (*move > MOVE_MAX) {
		*move = MOVE_MAX;
	}

	player.prev_ball_pos = *ball_pos;
	player.prev_paddle_pos = *paddle_pos;
}

player_api_t PLAYER_NAME = {
	.init = init,
	.move = move,
};
