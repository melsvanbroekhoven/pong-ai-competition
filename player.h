typedef struct {
	unsigned int x, y;	 /* top-left position on the screen */
} position_t;

typedef struct {
	unsigned int width, height;	 /* top-left position on the screen */
} object_size_t;

typedef enum {
	MOVE_UP,
	MOVE_DOWN,
	MOVE_NONE
} move_e;

typedef enum {
	PLAYER_LEFT,
	PLAYER_RIGHT
} player_side_e;

typedef struct player_api {
	void (*init)(player_side_e, object_size_t *ball_size, object_size_t *paddle_size);
	void (*move)(position_t *ball_pos, position_t *paddle_pos, move_e *out_move);
} player_api_t;


