#include <stdlib.h>
#include "linked_list.h"

/**
 * 创建链表的头节点并返回
 */
List *ll_create_list(void)
{
  List *list = malloc(sizeof(List));
  if (list == NULL) {
    return NULL;
  }

  list->size = 0;
  list->head = NULL;
  list->tail = NULL;

  return list;
}

/**
 * 在链表的尾部追加节点
 */
void ll_append_list_node(List *list, ListNode *node)
{
  if (list == NULL) { return; }

  if(list->head == NULL)
  {
    list->head = node;
    list->tail = node;

	node->next = NULL;
	node->prev = NULL;
  } else 
  {
    ListNode *tail = list->tail;
    tail->next = node;
	node->prev = tail;
	node->next = NULL;

    list->tail = node;
  }

  list->size += 1;
}

/**
 * 在链表的头部插入节点，注意链表带头节点
 */
void ll_prepend_list_node(List *list, ListNode *node)
{
  if (list == NULL) { return; }

  if(list->head == NULL)
  {
    list->head = node;
    list->tail = node;

	node->next = NULL;
	node->prev = NULL;
  } else 
  {
    node->next = list->head;
	node->prev = NULL;
	list->head->prev = node;

    list->head = node;
  }

  list->size += 1;
}

/**
 * 销毁链表，同时销毁链表头节点
 */
void ll_free_list(List *list)
{
  int size = list->size;
  ListNode *current, *next;

  current = list->head;

  while(size--)
  {
    next = current->next;
    free(current);

    current = next;
  }

  free(list);
}
