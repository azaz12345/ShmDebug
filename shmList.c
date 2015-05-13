#include <stdio.h>
#include <stdlib.h>
#include "shmList.h"

int list_init(list_t *list)
{
	return 1;
}


int list_insert(list_t *list, listNode *node)
{
	if(list==NULL || node==NULL)
	{
		return 0;
	}
	if(list->head == NULL && list->last == NULL)
	{
		node->pre = NULL;
		node->next = NULL;
		list->last = node;
		list->head = node;
		list->length++;
		return 1;
	}
	node->pre = NULL;
	list->head->pre = node;
	node->next = list->head;
	list->head = node;
	//printf("event type:%d \n", ((event_t *)queue->head->element)->type);
	list->length++;
	
	return 1;
}

int list_rm(list_t *list, listNode *node)
{
	struct listNode *pre, *next;

	if(list->head == NULL && list->last == NULL)
	{
		return 0;
	}

	if(list->length==0)
	{
		return 0;
	}

	pre = node->pre;
	next = node->next;

	if(pre != NULL)
	{
		pre->next = next;
	}

	if(next != NULL)
	{
		next->pre = pre;
	}

	/*update head and last of queue*/
	if(pre == NULL && next == NULL)/*ONLY one element case*/
	{
		list->head = NULL;
		list->last = NULL;
	}
	else if(pre != NULL && next == NULL)/*element is last case*/
	{
		list->last = pre;

		if(list->last == NULL)
		{
			list->head = NULL;
		}
	}
	else if(pre == NULL && next != NULL)/*element is head case*/
	{
		list->head = next;

		if(list->head == NULL)
		{
			list->last = NULL;
		}
	}
	else if(pre != NULL && next != NULL)/*element is between last and head case*/
	{
		//nothing
		;
	}

	list->length--;
	return 1;

}

int list_popLast(list_t *list, listNode **node)
{
	if(list->head == NULL && list->last == NULL)
	{
		return 0;
	}
	*node = (listNode *)list->last;

	list->last = list->last->pre;
	if(list->last == NULL)
	{
		list->head = NULL;
	}
	list->length--;

	return 1;

}

int list_popHead(list_t *list, listNode **node)
{
	if(list->head == NULL && list->last == NULL)
	{
		return 0;
	}
	*node = (listNode *)list->head;

	list->head = list->head->next;
	if(list->head == NULL)
	{
		list->last = NULL;
	}
	list->length--;

	return 1;
}
