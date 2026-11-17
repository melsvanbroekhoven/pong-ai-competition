typedef struct {
	int x, y;	 /* top-left position on the screen */
} position_t;

typedef struct {
	unsigned int width, height;	 /* top-left position on the screen */
} object_size_t;

#define MOVE_MIN -4
#define MOVE_MAX 4
typedef int move_t; /* Min -4 (up), max +4 (down) */

typedef enum {
	PLAYER_LEFT,
	PLAYER_RIGHT
} player_side_e;

typedef struct player_api {
	void (*init)(player_side_e, object_size_t *ball_size, object_size_t *paddle_size);
	void (*move)(position_t *ball_pos, position_t *paddle_pos, move_t *out_move);
} player_api_t;


