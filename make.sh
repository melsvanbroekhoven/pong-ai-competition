gcc -c left/ai.c -DPLAYER_NAME=player_left -o left_ai.o
gcc -c right/ai.c -DPLAYER_NAME=player_right -o right_ai.o
#gcc -c `sdl2-config --cflags --libs` pong.c pong.o
gcc pong.c left_ai.o right_ai.o `sdl2-config --cflags --libs` -o pong
