#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <time.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

#define NROWS 4
#define NCOLS NROWS

typedef unsigned int tile_t;

/**
 * 存储游戏操作次数、得分、面板数据
 */
struct game_t {
	int turns, score;
	tile_t board[NROWS][NCOLS];
};

/**
 * 在game_t的board中寻找一个新的位置放置元素1或者2（1代表2^1，2代表2^2)，如果
 * 面板已经填满，返回-1.
 */
int place_tile(struct game_t *game)
{
	// game_t中board是二维数组，二维数组也是线性存储，lboard表示linearboard
	tile_t *lboard = (tile_t *)game->board;
	int i, num_zeros = 0;

	// game_t结构体使用前初始化为0，board中为0的位置代表空位置
	for (i = 0; i < NROWS * NCOLS; i++) {
		num_zeros += lboard[i] ? 0 : 1;
	}

	if (!num_zeros) {
		return -1;
	}

	// 选择插入元素的位置
	int loc = random() % num_zeros;

	// 利用伪随机数生成新元素
	for (i = 0; i < NROWS * NCOLS; i++) {
		if (!lboard[i] && !(loc--)) {
			lboard[i] = random() % 10 ? 1 : 2;
			return 0;
		}
	}
	return -1;
}

/**
 * 将元素数据打印出来，注意，将面板中元素值i转成2的幂的形式
 */
void print_tile(int tile)
{
	if (tile) {
		if (tile < 6)
			attron(A_BOLD);
		int pair = COLOR_PAIR(1 + (tile % 6));
		attron(pair);
		printw("%4d", 1 << tile);
		attroff(pair);
		attroff(A_BOLD);
	}
	else {
		printw("   .");
	}
}

/**
 * 打印游戏状态
 */
void print_game(struct game_t *game)
{
	int r, c;
	move(0, 0);
	printw("Score: %6d  Turns: %4d", game->score, game->turns);
	for (r = 0; r < NROWS; r++) {
		for (c = 0; c < NCOLS; c++) {
			move(r + 2, 5 * c);
			print_tile(game->board[r][c]);
		}
	}

	refresh();
}

/**
 * 向左合并相同数值的元素
 */
int combine_left(struct game_t *game, tile_t row[NCOLS])
{
	int c, did_combine = 0;
	for (c = 1; c < NCOLS; c++) {
		if (row[c] && row[c-1] == row[c]) {
			row[c-1]++;
			row[c] = 0;
			game->score += 1 << (row[c-1] - 1);
			did_combine = 1;
		}
	}
	return did_combine;
}

/**
 * 向左部进行压缩，将空白位置用右侧非空白数字填充
 */
int deflate_left(tile_t row[NCOLS])
{
	tile_t buf[NCOLS] = {0};
	tile_t *out = buf;
	int did_deflate = 0;

	int in;
	for (in = 0; in < NCOLS; in++) {
		if (row[in] != 0) {
			*out++ = row[in];
			did_deflate |= buf[in] != row[in];
		}
	}
	memcpy(row, buf, sizeof(buf));
	return did_deflate;
}

/**
 * 将game_t结构中的board顺时针旋转，以后向后向左向上向下的操作都可以通过向左的
 * 操作配合顺时针旋转的组合来实现
 */
void rotate_clockwise(struct game_t *game)
{
	tile_t buf[NROWS][NCOLS];
	memcpy(buf, game->board, sizeof(game->board));

	int r, c;
	for (r = 0; r < NROWS; r++) {
		for (c = 0; c < NCOLS; c++) {
			game->board[r][c] = buf[NCOLS - c - 1][r];
		}
	}
}

/**
 * 元素向左移动操作
 */
void move_left(struct game_t *game)
{
	int r, ret = 0;
	for (r = 0; r < NROWS; r++) {
		tile_t *row = &game->board[r][0];
		// 因为是每行4个元素，多向左移动一次合并两次
		ret |= deflate_left(row);
		ret |= combine_left(game, row);
		ret |= deflate_left(row);
	}

	game->turns += ret;
}

/**
 * 元素向右移动操作
 */
void move_right(struct game_t *game)
{
	rotate_clockwise(game);
	rotate_clockwise(game);
	move_left(game);
	rotate_clockwise(game);
	rotate_clockwise(game);
}

/**
 * 元素向上移动操作
 */
void move_up(struct game_t *game)
{
	rotate_clockwise(game);
	rotate_clockwise(game);
	rotate_clockwise(game);
	move_left(game);
	rotate_clockwise(game);
}

/**
 * 元素向下移动操作
 */
void move_down(struct game_t *game)
{
	rotate_clockwise(game);
	move_left(game);
	rotate_clockwise(game);
	rotate_clockwise(game);
	rotate_clockwise(game);
}

/**
 * 检查游戏是否已经结束
 */
int lose_game(struct game_t test_game)
{
	int start_turns = test_game.turns;
	move_left(&test_game);
	move_up(&test_game);
	move_down(&test_game);
	move_right(&test_game);
	// 如果面板中的数据可以执行上下左右中某个方向的移动，则就返回0，表示未结束
	return test_game.turns == start_turns;
}

/**
 * curses相关初始化操作
 */
void init_curses()
{
	int bg = 0;

	// 对屏幕进行初始化，如果调用成功，则返回一个WINDOW指针到stdscr
	initscr();
	// 要使用颜色的话，必须在任何颜色相关操作之前，调用如下start_color例程
	start_color();
	// 禁止行缓冲，使用户键入的字符立即传送给应用程序
	cbreak();
	// 用户键入的字符通过getch读取时禁止回显
	noecho();
	// 在stdscr指向的窗口中，激活终端的键区，方便处理功能键
	keypad(stdscr,TRUE);
	// 清空屏幕内容，准备重画
	clear();
	// 重画屏幕内容
	refresh();
	// 将光标设为不可见
	curs_set(0);

	// 颜色搭配
	bg = use_default_colors() == OK ? -1 : 0;
	init_pair(1, COLOR_RED, bg);
	init_pair(2, COLOR_GREEN, bg);
	init_pair(3, COLOR_YELLOW, bg);
	init_pair(4, COLOR_BLUE, bg);
	init_pair(5, COLOR_MAGENTA, bg);
	init_pair(6, COLOR_CYAN, bg);
}

/**
 * 返回游戏面板中的最大数值
 */
int max_tile(tile_t *lboard)
{
	int i, ret = 0;
	for (i = 0; i < NROWS * NCOLS; i++) {
		ret = max(ret, lboard[i]);
	}
	return ret;
}

int main()
{
	// curses初始化操作
	init_curses();

	const char *exit_msg = "";
	srandom(time(NULL));

	// game_t中存储着游戏相关的状态
	struct game_t game = {0};
	int last_turn = game.turns;

	// 随机地在面板上找两个位置放置两个板子，板子中数值为1或者2
	place_tile(&game);
	place_tile(&game);

	while (1) {
		// 将游戏状态信息打印出来
		print_game(&game);

		// 检查当前游戏是否已经满足终止的条件
		if (lose_game(game)) {
			exit_msg = "lost";
			goto lose;
		}

		// 更新行走的步数
		last_turn = game.turns;

		// 检测用户当前键入的控制字符，并执行处理
		switch (getch()) {
			case 'h': 
			case KEY_LEFT: 
				move_left(&game); 
				break;
			case 'j': 
			case KEY_DOWN: 
				move_down(&game); 
				break;
			case 'k': 
			case KEY_UP: 
				move_up(&game); break;
			case 'l': 
			case KEY_RIGHT: 
				move_right(&game); break;
			case 'q':
				exit_msg = "quit";
				goto end;
		}

		if (last_turn != game.turns)
			place_tile(&game);
	}

lose:
	// 游戏失败
	move(7, 0);
	printw("You lose! Press q to quit.");
	while (getch() != 'q')
		;
end:
	// 退出操作
	endwin();
	printf("You %s after scoring %d points in %d turns, "
		"with largest tile %d\n",
		exit_msg, game.score, game.turns,
		1 << max_tile((tile_t *)game.board));
	return 0;
}

