CC = gcc
CFLAGS = -std=c99 -O2 -Wall -Wextra

OBJS = main.o board.o logic.o
TARGET = game

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# 単体テスト用
b01: 01_board_render.c
	$(CC) $(CFLAGS) 01_board_render.c -o b01
b02: 02_match_generic.c
	$(CC) $(CFLAGS) 02_match_generic.c -o b02
b03: 03_gravity_param.c
	$(CC) $(CFLAGS) 03_gravity_param.c -o b03
b04: 04_refill_chain.c
	$(CC) $(CFLAGS) 04_refill_chain.c -o b04
b05: 05_fps_loop.c
	$(CC) $(CFLAGS) 05_fps_loop.c -o b05
b06: 06_swap_play.c
	$(CC) $(CFLAGS) 06_swap_play.c -o b06
