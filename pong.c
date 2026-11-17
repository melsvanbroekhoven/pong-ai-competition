#include <errno.h>

// Using libs SDL, glibc
#include <SDL.h> //SDL version 2.0
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "player.h"

#define SCREEN_WIDTH 640	// window height
#define SCREEN_HEIGHT 480 // window width

// function prototypes
// initilise SDL
int init(int w, int h, int argc, char *args[]);

typedef struct ball_s {
	int x, y;	 /* position on the screen */
	int w, h;	 // ball width and height
	int dx, dy; /* movement vector */
} ball_t;

typedef struct paddle {
	int x, y;
	int w, h;
} paddle_t;

// Program globals
static ball_t ball;
static paddle_t paddle[2];
int score[] = {0, 0};
int width, height; // used if fullscreen

SDL_Window *window = NULL; // The window we'll be rendering to
SDL_Renderer *renderer;		// The renderer SDL will use to draw to the screen

// surfaces
static SDL_Surface *screen;
static SDL_Surface *title;
static SDL_Surface *numbermap;
static SDL_Surface *end;

// textures
SDL_Texture *screen_texture;

// inisilise starting position and sizes of game elemements
static void init_game() {

	ball.x = screen->w / 2;
	ball.y = screen->h / 2;
	ball.w = 10;
	ball.h = 10;
	ball.dy = 1;
	ball.dx = 1;

	paddle[0].x = 20;
	paddle[0].y = screen->h / 2 - 50;
	paddle[0].w = 10;
	paddle[0].h = 50;

	paddle[1].x = screen->w - 20 - 10;
	paddle[1].y = screen->h / 2 - 50;
	paddle[1].w = 10;
	paddle[1].h = 50;
}

int check_score() {

	int i;

	// loop through player scores
	for (i = 0; i < 2; i++) {

		// check if score is @ the score win limit
		if (score[i] == 10) {

			// reset scores
			score[0] = 0;
			score[1] = 0;

			// return 1 if player 1 score @ limit
			if (i == 0) {

				return 1;

				// return 2 if player 2 score is @ limit
			} else {

				return 2;
			}
		}
	}

	// return 0 if no one has reached a score of 10 yet
	return 0;
}

static int cross(int32_t *a, int32_t *b)
{
	//printf("cross([%d, %d], [%d, %d]) = %d\n", a[0], a[1], b[0], b[1], a[0]*b[1] - a[1]*b[0]); 
	return a[0]*b[1] - a[1]*b[0];
}

static int get_intersection(position_t *a0, position_t *b0, position_t *a1, position_t *b1)
{
	// Calculate collisions points between line segments
	int32_t r[2], s[2], tmp[2];
	r[0] = b0->x - a0->x;
	r[1] = b0->y - a0->y;
	s[0] = b1->x - a1->x;
	s[1] = b1->y - a1->y;

	//printf("(%d,%d)-(%d,%d) and (%d,%d)-(%d,%d)\n",
	//		a0->x, a0->y, b0->x, b0->y, a1->x, a1->y, b1->x, b1->y); 
	int denom = cross(r, s);

	if (denom == 0) {
		// Parallel - no collision
		return 0;
	}

	tmp[0] = a1->x - a0->x;
	tmp[1] = a1->y - a0->y;
	double t = (double)cross(tmp, s) / (double)denom;
	double u = (double)cross(tmp, r) / (double)denom;

	//printf("denom=%d\n", denom);
	//printf("t=%f, u=%f\n", t, u);
	if ((t < 0 || t > 1) || (u < 0 || u > 1)) {
		// Outside of segments
		return 0;
	}

	return 1;
}

// if return value is 1 collision occured. if return is 0, no collision.
int check_collision(ball_t ball, paddle_t paddle)
{
	// Points for line segments a-b
	position_t ball_pos_a0, ball_pos_a1, ball_pos_b0, ball_pos_b1,
			   paddle_pos_a, paddle_pos_b;

	if (ball.dx > 0) {
		// Moving right, use right points of ball
		// top-right:
		ball_pos_a0.x = ball.x + ball.w;
		ball_pos_a0.y = ball.y;
		// bottom-right:
		ball_pos_a1.x = ball.x + ball.w;
		ball_pos_a1.y = ball.y + ball.h;
		// Use left points of paddle
		paddle_pos_a.x = paddle.x;
		paddle_pos_a.y = paddle.y;
		paddle_pos_b.x = paddle.x;
		paddle_pos_b.y = paddle.y + paddle.h;
	} else {
		// Moving left, use left points of ball
		// top-left:
		ball_pos_a0.x = ball.x;
		ball_pos_a0.y = ball.y;
		// bottom-left:
		ball_pos_a1.x = ball.x;
		ball_pos_a1.y = ball.y + ball.h;
		// Use right points of paddle
		paddle_pos_a.x = paddle.x + paddle.w;
		paddle_pos_a.y = paddle.y;
		paddle_pos_b.x = paddle.x + paddle.w;
		paddle_pos_b.y = paddle.y + paddle.h;
	}
	// Calculate next positions of ball points
	ball_pos_b0.x = ball_pos_a0.x + ball.dx;
	ball_pos_b0.y = ball_pos_a0.y + ball.dy;
	ball_pos_b1.x = ball_pos_a1.x + ball.dx;
	ball_pos_b1.y = ball_pos_a1.y + ball.dy;

	if (get_intersection(&ball_pos_a0, &ball_pos_b0, &paddle_pos_a, &paddle_pos_b)) {
		return 1;
	}
	if (get_intersection(&ball_pos_a1, &ball_pos_b1, &paddle_pos_a, &paddle_pos_b)) {
		return 1;
	}
	return 0;
}

/* This routine moves each ball by its motion vector. */
static void move_ball() {
	// check for collision with the paddle
	for (int i = 0; i < 2; i++) {
		int c = check_collision(ball, paddle[i]);

		if (c != 1) {
			// No collision
			continue;
		}
		printf("HIT\n");

		// change ball direction
		ball.dx = -ball.dx;

		// change ball angle based on where on the paddle it hit
		int hit_pos = (paddle[i].y + paddle[i].h) - ball.y;

		if (hit_pos >= 0 && hit_pos < 7) {
			ball.dy = 4;
		}

		if (hit_pos >= 7 && hit_pos < 14) {
			ball.dy = 3;
		}

		if (hit_pos >= 14 && hit_pos < 21) {
			ball.dy = 2;
		}

		if (hit_pos >= 21 && hit_pos < 28) {
			ball.dy = 1;
		}

		if (hit_pos >= 28 && hit_pos < 32) {
			ball.dy = 0;
		}

		if (hit_pos >= 32 && hit_pos < 39) {
			ball.dy = -1;
		}

		if (hit_pos >= 39 && hit_pos < 46) {
			ball.dy = -2;
		}

		if (hit_pos >= 46 && hit_pos < 53) {
			ball.dy = -3;
		}

		if (hit_pos >= 53 && hit_pos <= 60) {
			ball.dy = -4;
		}

		// collision detected, speed up
		if (ball.dx < 0) {
			// ball moving left
			ball.dx -= 1;
		} else {
			// ball moving right
			ball.dx += 1;
		}
	}

	ball.x += ball.dx;
	ball.y += ball.dy;

	// ball moving right
	if (ball.dx > 0) {
		// teleport ball to avoid mutli collision glitch
		if (ball.x < 30) {
			ball.x = 30;
		}
	} else {
		// teleport ball to avoid mutli collision glitch
		if (ball.x > 600) {
			ball.x = 600;
		}
	}
	if (ball.y < 0 || ball.y > screen->h - ball.h) {
		ball.dy = -ball.dy;
	}
	if (ball.y < 0) {
		ball.y = 0;
	}
	if (ball.y > screen->h - ball.h) {
		ball.y = screen->h - ball.h;
	}

	/* Turn the ball around if it hits the edge of the screen. */
	if (ball.x < 0) {
		score[1] += 1;
		printf("Player Right scored!\n");
		SDL_Delay(1000);
		init_game();
	}

	if (ball.x > screen->w - ball.w) {
		score[0] += 1;
		printf("Player Left scored!\n");
		SDL_Delay(1000);
		init_game();
	}

}

static void move_paddle_ai(paddle_t *paddle, int player)
{
	int center = paddle->y + 25;
	int screen_center = screen->h / 2 - 25;
	int ball_speed = ball.dy;

	if (ball_speed < 0) {
		ball_speed = -ball_speed;
	}

	// ball moving right
	if ((ball.dx > 0 && player == 0) || (ball.dx < 0 && player == 1)) {
		// return to center position
		if (center < screen_center) {
			paddle->y += ball_speed;
		} else {
			paddle->y -= ball_speed;
		}
		// ball moving left
	} else {
		// ball moving down
		if (ball.dy > 0) {
			 if (ball.y > center) {
				paddle->y += ball_speed;
			} else {
				paddle->y -= ball_speed;
			}
		}

		// ball moving up
		if (ball.dy < 0) {
			if (ball.y < center) {
				paddle->y -= ball_speed;
			} else {
				paddle->y += ball_speed;
			}
		}

		// ball moving stright across
		if (ball.dy == 0) {
			if (ball.y < center) {
				paddle->y -= 5;
			} else {
				paddle->y += 5;
			}
		}
	}
}

static void move_player_paddle(paddle_t *paddle, move_t move)
{
	if (move > MOVE_MAX || move < MOVE_MIN) {
		/* Ignore move */
		printf("Move %d out of bounds!\n", move);
		return;
	}
	paddle->y += move;
}

static void move_paddle(int d) {
	// if the down arrow is pressed move paddle down
	if (d == 0) {

		if (paddle[1].y >= screen->h - paddle[1].h) {

			paddle[1].y = screen->h - paddle[1].h;

		} else {

			paddle[1].y += 5;
		}
	}

	// if the up arrow is pressed move paddle up
	if (d == 1) {

		if (paddle[1].y <= 0) {

			paddle[1].y = 0;

		} else {

			paddle[1].y -= 5;
		}
	}
}

static void draw_game_over(int p) {
	SDL_Rect p1;
	SDL_Rect p2;
	SDL_Rect cpu;
	SDL_Rect dest;

	p1.x = 0;
	p1.y = 0;
	p1.w = end->w;
	p1.h = 75;

	p2.x = 0;
	p2.y = 75;
	p2.w = end->w;
	p2.h = 75;

	cpu.x = 0;
	cpu.y = 150;
	cpu.w = end->w;
	cpu.h = 75;

	dest.x = (screen->w / 2) - (end->w / 2);
	dest.y = (screen->h / 2) - (75 / 2);
	dest.w = end->w;
	dest.h = 75;

	switch (p) {

	case 1:
		SDL_BlitSurface(end, &p1, screen, &dest);
		break;
	case 2:
		SDL_BlitSurface(end, &p2, screen, &dest);
		break;
	default:
		SDL_BlitSurface(end, &cpu, screen, &dest);
	}
}

static void draw_menu() {

	SDL_Rect src;
	SDL_Rect dest;

	src.x = 0;
	src.y = 0;
	src.w = title->w;
	src.h = title->h;

	dest.x = (screen->w / 2) - (src.w / 2);
	dest.y = (screen->h / 2) - (src.h / 2);
	dest.w = title->w;
	dest.h = title->h;

	SDL_BlitSurface(title, &src, screen, &dest);
}

static void draw_background() {

	SDL_Rect src;

	// draw bg with net
	src.x = 0;
	src.y = 0;
	src.w = screen->w;
	src.h = screen->h;

	// draw the backgorund
	// int r = SDL_FillRect(screen,&src,0);

	// if (r !=0){

	//	printf("fill rectangle faliled in func draw_background()");
	//}
}

static void draw_net() {

	SDL_Rect net;

	net.x = screen->w / 2;
	net.y = 20;
	net.w = 5;
	net.h = 15;

	// draw the net
	int i, r;

	for (i = 0; i < 15; i++) {

		r = SDL_FillRect(screen, &net, 0xffffffff);

		if (r != 0) {

			printf("fill rectangle faliled in func draw_net()");
		}

		net.y = net.y + 30;
	}
}

static void draw_ball() {

	SDL_Rect src;

	src.x = ball.x;
	src.y = ball.y;
	src.w = ball.w;
	src.h = ball.h;

	int r = SDL_FillRect(screen, &src, 0xffffffff);

	if (r != 0) {

		printf("fill rectangle faliled in func drawball()");
	}
}

static void draw_paddle() {

	SDL_Rect src;
	int i;

	for (i = 0; i < 2; i++) {

		src.x = paddle[i].x;
		src.y = paddle[i].y;
		src.w = paddle[i].w;
		src.h = paddle[i].h;

		int r = SDL_FillRect(screen, &src, 0xffffffff);

		if (r != 0) {

			printf("fill rectangle faliled in func draw_paddle()");
		}
	}
}

static void draw_player_0_score() {

	SDL_Rect src;
	SDL_Rect dest;

	src.x = 0;
	src.y = 0;
	src.w = 64;
	src.h = 64;

	dest.x = (screen->w / 2) - src.w - 12; // 12 is just padding spacing
	dest.y = 0;
	dest.w = 64;
	dest.h = 64;

	if (score[0] > 0 && score[0] < 10) {

		src.x += src.w * score[0];
	}

	SDL_BlitSurface(numbermap, &src, screen, &dest);
}

static void draw_player_1_score() {

	SDL_Rect src;
	SDL_Rect dest;

	src.x = 0;
	src.y = 0;
	src.w = 64;
	src.h = 64;

	dest.x = (screen->w / 2) + 12;
	dest.y = 0;
	dest.w = 64;
	dest.h = 64;

	if (score[1] > 0 && score[1] < 10) {

		src.x += src.w * score[1];
	}

	SDL_BlitSurface(numbermap, &src, screen, &dest);
}


static void timespec_diff(struct timespec *prev_ts, struct timespec *cur_ts,
                   struct timespec *diff)
{
    if ((cur_ts->tv_nsec - prev_ts->tv_nsec) < 0) {
        diff->tv_sec = cur_ts->tv_sec - prev_ts->tv_sec - 1;
        diff->tv_nsec = cur_ts->tv_nsec - prev_ts->tv_nsec + 1000000000;
    } else {
        diff->tv_sec = cur_ts->tv_sec - prev_ts->tv_sec;
        diff->tv_nsec = cur_ts->tv_nsec - prev_ts->tv_nsec;
    }
}

static void timespec_add_ns(struct timespec *ts, uint64_t ns,
                   struct timespec *result)
{
	result->tv_nsec = ts->tv_nsec + ns;
	result->tv_sec = ts->tv_sec;
	if (result->tv_nsec > 1000000000) {
		result->tv_sec++;
		result->tv_nsec = (result->tv_nsec % 1000000000);
	}
}


#define FPS_TARGET 60
#define MOVE_BUDGET_US 4000

extern player_api_t player_left, player_right;

int main(int argc, char *args[])
{
	int framecount = 0;
	uint64_t diff_us;
	struct timespec prev_fps_ts, cur_ts, diff_ts, prev_frame_ts, next_frame_ts, move_start_ts, move_end_ts;

	// SDL Window setup
	if (init(SCREEN_WIDTH, SCREEN_HEIGHT, argc, args) == 1) {
		return 0;
	}

	SDL_GetWindowSize(window, &width, &height);

	int sleep = 0;
	int quit = 0;
	int state = 0;
	int r = 0;
	Uint32 next_game_tick = SDL_GetTicks();

	// Initialize the ball position data.
	init_game();

	object_size_t ball_size = {
		.width = ball.w,
		.height = ball.h,
	};
	object_size_t paddle_size = {
		.width = paddle[0].w,
		.height = paddle[0].h,
	};
	// Init players
	player_left.init(PLAYER_LEFT, &ball_size, &paddle_size);
	player_right.init(PLAYER_RIGHT, &ball_size, &paddle_size);

	clock_gettime(CLOCK_MONOTONIC, &prev_fps_ts);
	prev_frame_ts = prev_fps_ts;
	// render loop
	while (quit == 0) {
		// check for new events every frame
		SDL_PumpEvents();

		const Uint8 *keystate = SDL_GetKeyboardState(NULL);

		if (keystate[SDL_SCANCODE_ESCAPE]) {
			quit = 1;
		}

		if (keystate[SDL_SCANCODE_DOWN]) {
			move_paddle(0);
		}

		if (keystate[SDL_SCANCODE_UP]) {
			move_paddle(1);
		}

		// draw background
		SDL_RenderClear(renderer);
		SDL_FillRect(screen, NULL, 0x000000ff);

		// display main menu
		if (state == 0) {
			if (keystate[SDL_SCANCODE_SPACE]) {
				state = 1;
			}

			// draw menu
			draw_menu();

			// display gameover
		} else if (state == 2) {
			if (keystate[SDL_SCANCODE_SPACE]) {
				state = 0;
				// delay for a little bit so the space bar press dosnt get triggered
				// twice while the main menu is showing
				SDL_Delay(500);
			}

			// display gameover
			draw_game_over(r);
		} else if (state == 1) {
			// display the game

			// check score
			r = check_score();

			// if either player wins, change to game over state
			if (r == 1) {
				state = 2;
			} else if (r == 2) {
				state = 2;
			}

			// paddle ai movement
			position_t ball_position = {
				.x = ball.x,
				.y = ball.y,
			};
			move_t move;
			position_t left_paddle_position = {
				.x = paddle[0].x,
				.y = paddle[0].y,
			};
			clock_gettime(CLOCK_MONOTONIC, &move_start_ts);
			player_left.move(&ball_position, &left_paddle_position, &move);
			clock_gettime(CLOCK_MONOTONIC, &move_end_ts);
			timespec_diff(&move_start_ts, &move_end_ts, &diff_ts);
			if ((diff_ts.tv_sec * 1000000 + diff_ts.tv_nsec / 1000) >= MOVE_BUDGET_US) {
				printf("Player left exceeded time budget %ldus!\n", diff_ts.tv_sec * 1000000 + diff_ts.tv_nsec / 1000);
				score[1] += 1;
				printf("Player Right scored!\n");
				SDL_Delay(1000);
				init_game();
			}
			move_player_paddle(&paddle[0], move);

			position_t right_paddle_position = {
				.x = paddle[1].x,
				.y = paddle[1].y,
			};
			clock_gettime(CLOCK_MONOTONIC, &move_start_ts);
			player_right.move(&ball_position, &right_paddle_position, &move);
			clock_gettime(CLOCK_MONOTONIC, &move_end_ts);
			timespec_diff(&move_start_ts, &move_end_ts, &diff_ts);
			if ((diff_ts.tv_sec * 1000000 + diff_ts.tv_nsec / 1000) >= MOVE_BUDGET_US) {
				printf("Player right exceeded time budget %luus!\n", diff_ts.tv_sec * 1000000 + diff_ts.tv_nsec / 1000);
				score[0] += 1;
				printf("Player Left scored!\n");
				SDL_Delay(1000);
				init_game();
			}
			move_player_paddle(&paddle[1], move);

			//* Move the balls for the next frame.
			move_ball();

			// draw net
			draw_net();

			// draw paddles
			draw_paddle();

			//* Put the ball on the screen.
			draw_ball();

			// draw the score
			draw_player_0_score();

			// draw the score
			draw_player_1_score();
		}

		SDL_UpdateTexture(screen_texture, NULL, screen->pixels,
											screen->w * sizeof(Uint32));
		SDL_RenderCopy(renderer, screen_texture, NULL, NULL);

		// draw to the display
		SDL_RenderPresent(renderer);

		clock_gettime(CLOCK_MONOTONIC, &cur_ts);

		framecount++;

		// Check FPS counter
		timespec_diff(&prev_fps_ts, &cur_ts, &diff_ts);
		diff_us = diff_ts.tv_sec * 1000000 + diff_ts.tv_nsec / 1000;
		if ((diff_ts.tv_sec * 1000000 + diff_ts.tv_nsec / 1000) >= 1000000) {
			printf("FPS: %f\n", (float)(framecount * 1000000) / diff_us);
			prev_fps_ts = cur_ts;
			framecount = 0;
		}

		clock_gettime(CLOCK_MONOTONIC, &cur_ts);
		// Sleep until next frame
		timespec_add_ns(&prev_frame_ts, 1000000000/FPS_TARGET, &next_frame_ts);
		timespec_diff(&cur_ts, &next_frame_ts, &diff_ts);

		prev_frame_ts = next_frame_ts;
		struct timespec remaining_ts;
		while (nanosleep(&diff_ts, &remaining_ts) == -1) {
			int error = errno;
			if (error == EINTR) {
					diff_ts = remaining_ts;
			} else if (error == EINVAL) {
				// Frame overflow
				break;
			}
		}
	}

	// free loaded images
	SDL_FreeSurface(screen);
	SDL_FreeSurface(title);
	SDL_FreeSurface(numbermap);
	SDL_FreeSurface(end);

	// free renderer and all textures used with it
	SDL_DestroyRenderer(renderer);

	// Destroy window
	SDL_DestroyWindow(window);

	// Quit SDL subsystems
	SDL_Quit();

	return 0;
}

int init(int width, int height, int argc, char *args[]) {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());

		return 1;
	}

	int i;

	for (i = 0; i < argc; i++) {

		// Create window
		if (strcmp(args[i], "-f")) {

			SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN,
																	&window, &renderer);

		} else {

			SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
																	SDL_WINDOW_FULLSCREEN_DESKTOP, &window,
																	&renderer);
		}
	}

	if (window == NULL) {

		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());

		return 1;
	}

	// create the screen sruface where all the elemnts will be drawn onto (ball,
	// paddles, net etc)
	screen = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32,
																					SDL_PIXELFORMAT_RGBA32);

	if (screen == NULL) {

		printf("Could not create the screen surfce! SDL_Error: %s\n",
					 SDL_GetError());

		return 1;
	}

	// create the screen texture to render the screen surface to the actual
	// display
	screen_texture = SDL_CreateTextureFromSurface(renderer, screen);

	if (screen_texture == NULL) {

		printf("Could not create the screen_texture! SDL_Error: %s\n",
					 SDL_GetError());

		return 1;
	}

	// Load the title image
	title = SDL_LoadBMP("title.bmp");

	if (title == NULL) {

		printf("Could not Load title image! SDL_Error: %s\n", SDL_GetError());

		return 1;
	}

	// Load the numbermap image
	numbermap = SDL_LoadBMP("numbermap.bmp");

	if (numbermap == NULL) {

		printf("Could not Load numbermap image! SDL_Error: %s\n", SDL_GetError());

		return 1;
	}

	// Load the gameover image
	end = SDL_LoadBMP("gameover.bmp");

	if (end == NULL) {

		printf("Could not Load title image! SDL_Error: %s\n", SDL_GetError());

		return 1;
	}

	// Set the title colourkey.
	Uint32 colorkey = SDL_MapRGB(title->format, 255, 0, 255);
	SDL_SetColorKey(title, SDL_TRUE, colorkey);
	SDL_SetColorKey(numbermap, SDL_TRUE, colorkey);

	return 0;
}
