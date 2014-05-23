#include <ncurses.h>
#include "defines.h"
#include "linked_list.h"
#include "snake.h"

int moveX = 0;
int moveY = 0;

void appendSnakePartWithPoint(Snake *snake, Point *position);
int getPositionInBound(int n, int max_bound);

/**
 * 创建蛇，蛇初始尺寸为size个元素
 */
Snake *snake_create(int size)
{
   int i;
   // snake是链表的头节点
   Snake *snake = ll_create_list();

   for (i = 0; i < size; i++) {
     Point *position = malloc(sizeof(Point));
     position->x = size-i;
     position->y = 1;

     appendSnakePartWithPoint(snake, position);
   }

   return snake;
}

/**
 * 蛇移动，除了蛇的头部以外，部位i+1的位置修改为部位i的位置
 */
void snake_move(Snake *snake, Direction direction, int max_width, int max_height)
{
	// 除了蛇的头部意外，蛇的后续部位的坐标替换成前一部位的坐标，蛇头部的坐标
	// 根据用户的输入，单独做调整
	ListNode *tail_node = snake->tail;
	ListNode *pre_tail_node = tail_node->prev;

	while(pre_tail_node!=NULL) {
		Point *tail_position = (Point *)tail_node->value;
		Point *pre_tail_position = (Point *)pre_tail_node->value;

		tail_position->x = pre_tail_position->x;
		tail_position->y = pre_tail_position->y;
		
		tail_node = pre_tail_node;
		pre_tail_node = tail_node->prev;
	}

  switch (direction) {
    case RIGHT:
      moveX = 1;
	  moveY = 0;
      break;
    case LEFT:
      moveX = -1;
	  moveY = 0;
      break;
    case UP:
	  moveX = 0;
      moveY = -1;
      break;
    case DOWN:
	  moveX = 0;
      moveY = 1;
      break;
	case KEEP:
	  moveX = 0;
	  moveY = 0;
  }

  // 蛇的头部的位置单独进行处理
  ListNode *snake_head_node = snake->head;
  Point *snake_head_part = (Point *)snake_head_node->value;

  snake_head_part->x += moveX;
  snake_head_part->y += moveY;

  snake_head_part->x = getPositionInBound(snake_head_part->x, max_width);
  snake_head_part->y = getPositionInBound(snake_head_part->y, max_height);
}

/**
 * 蛇吃了食物之后，会增长，在蛇的尾部添加元素
 */
void snake_add_part_to_tail(Snake *snake)
{

  ListNode *tail = snake->tail;
  ListNode *pre_tail = tail->prev;

  Point *tail_position = (Point *)tail->value;
  Point *pre_tail_position = (Point *)pre_tail->value;
  
  int x_change = tail_position->x - pre_tail_position->x;
  int y_change = tail_position->y - pre_tail_position->y;

  Point *new_part_position = malloc(sizeof(Point));
  new_part_position->x = tail_position->x + x_change;
  new_part_position->y = tail_position->y + y_change;

  ListNode *new_part_node = malloc(sizeof(ListNode));
  new_part_node->value = (void *)new_part_position;

  ll_append_list_node(snake, new_part_node);
}

/**
 * 在蛇snake后端添加新元素，代表蛇的增长
 */
void appendSnakePartWithPoint(Snake *snake, Point *position)
{
     ListNode *node = malloc(sizeof(ListNode));
     node->value = (void *)position;
     ll_append_list_node(snake, node);
}

/**
 * 蛇的身体部位的位置可能超出边框，将其坐标进行转换，
 * 始终处于边框内
 */
int getPositionInBound(int n, int max_bound)
{
  // if move beyond min bound move to the max bound
  if (n < 0) {
    n = max_bound;
  }
  // check that dont go over max bound
  else
  {
    n = n % max_bound;
  }

  return n;
}
