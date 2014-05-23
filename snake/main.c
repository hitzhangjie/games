#include <stdlib.h>
#include <ncurses.h>
#include <stdio.h>
#include <time.h>
#include "linked_list.h"
#include "defines.h"
#include "snake.h"
#include <unistd.h>

#define WORLD_WIDTH 50
#define WORLD_HEIGHT 20
#define TICKRATE 200

#define SNAKE_INITIAL_LENGTH 3

extern int moveX;
extern int moveY;

char head;

WINDOW *create_world();
Direction direction_for_key(int ch);
int are_points_equal(Point a, Point b);
void draw_snake(WINDOW *, Snake *snake);
Point new_food_position(void);
 
int main(int argc, char *argv[]) {
	// 创建一个世界
    WINDOW *world;
	// 创建一条蛇
    Snake *snake;
	// 创建食物所在位置坐标
    Point food_point;
	// 初始移动方向为向右
    Direction direction = RIGHT;

    srand(time(NULL));
 
	// curses库初始化
    initscr();					/* 初始化屏幕							*/
    noecho();					/* 禁止输入字符getch读取回显			*/
    cbreak();					/* 关闭行缓冲							*/
    timeout(TICKRATE);			/* 每隔TICKRATE的时间检测一次窗口读操作 */
    keypad(stdscr, TRUE);		/* 开启键盘的键区，可以使用方向键		*/
    curs_set(0);				/* 禁止显示鼠标指针						*/
    
    refresh();					/* 刷新屏幕，准备重画					*/
 
    // 初始化世界
    world = create_world();
	// 以符号*界定世界窗口的边界
    box(world, '|' , '-');

	// 将世界窗口显示在终端上
    wrefresh(world);

    // 蛇初始化
    snake = snake_create(SNAKE_INITIAL_LENGTH);

	// 食物位置初始化
    food_point = new_food_position();
 
    int ch;
	// 因为没TICKRATE检测一次，并且设置了nodelay模式，getch不会
	// 阻塞，每隔TICKRATE下述循环将被执行一次
	while ((ch = getch()) != 'q')
    {
		// 每当检测到用户输入，重画世界
		wclear(world);
		box(world, '|' , '-');

		// 蛇移动，修改蛇的各个部分移动之后的坐标值
		snake_move(snake, direction, WORLD_WIDTH, WORLD_HEIGHT);

		Point *snake_head = (Point *)snake->head->value;
		// 蛇如果吃掉了食物，在新位置生成新的食物
		if (are_points_equal(food_point, *snake_head)) {
			food_point = new_food_position();
			snake_add_part_to_tail(snake);
		}

      // 绘制出蛇以及食物
      mvwaddch(world, food_point.y, food_point.x, '$');
      draw_snake(world, snake);

	  // 将当前绘制的窗口显示到终端
      wrefresh(world);

	  // 在nodelay模式下，getch读取不到字符则返回ERR
      if(ch != ERR) {
		// 修正移动方向
        direction = direction_for_key(ch);
      }
    }
 
    ll_free_list(snake);
    delwin(world);

    endwin();
 
    return 0;
}

/**
 * 创建世界
 */
WINDOW *create_world()
{
  // COLS,LINES代表屏幕的列树和行数
  int offsetX = (COLS - WORLD_WIDTH) / 2;
  int offsetY = (LINES - WORLD_HEIGHT) / 2;

  // 在指定的坐标(offsetX,offsetY)处创建世界
  return newwin(WORLD_HEIGHT, WORLD_WIDTH, offsetY, offsetX);
}

/**
 * 根据ch判断蛇移动的方向
 */
Direction direction_for_key(int ch)
{
  switch(ch) {
    case KEY_UP: 
    case 'k':
      return UP;
    case KEY_DOWN: 
	case 'j':
      return DOWN;
    case KEY_RIGHT: 
	case 'l':
      return RIGHT;
	case KEY_LEFT:
	case 'h':
	  return LEFT;
	// keep walking straight
    default:
      return KEEP;
  }
}

/**
 * 返回下次食物出现的位置
 */
Point new_food_position()
{
  Point food_position;
  food_position.x = (rand() % WORLD_WIDTH-1) + 1;
  food_position.y = (rand() % WORLD_HEIGHT-1) + 1;

  return food_position;
}

/**
 * 判断两个点的坐标是否相等
 */
int are_points_equal(Point a, Point b)
{
  if (a.x == b.x && a.y == b.y) {
    return 1;
  }

  return 0;
}

/**
 * 绘制蛇
 */
void draw_snake(WINDOW *win, Snake *snake)
{
  ListNode *snake_part = snake->head;

  while(snake_part != NULL)
  {
    Point *snake_part_position = (Point *)snake_part->value;

	// 蛇的头部和身体其他部位用不同的字符进行绘制
	if(snake_part==snake->head) 
	{
		if(moveX>0)
		{
			mvwaddch(win, snake_part_position->y, snake_part_position->x, '>');
			head = '>';
		}
		else if(moveX<0)
		{
			mvwaddch(win, snake_part_position->y, snake_part_position->x, '<');
			head = '<';
		}
		else if(moveY>0)
		{
			mvwaddch(win, snake_part_position->y, snake_part_position->x, 'v');
			head = 'v';
		}
		else if(moveY<0)
		{
			mvwaddch(win, snake_part_position->y, snake_part_position->x, '^');
			head = '^';
		}
		else if(moveX==0 && moveY==0)
			mvwaddch(win, snake_part_position->y, snake_part_position->x, head);
	}
	else
	    mvwaddch(win, snake_part_position->y, snake_part_position->x, 'o');

    snake_part = snake_part->next;
  }
}
