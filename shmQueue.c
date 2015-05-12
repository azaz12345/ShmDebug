#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shmQueue.h"
#include "shmMalloc.h"

int queue_create(shmQueue **queue)
{
	*queue = dbgMem_malloc(sizeof(shmQueue));
	if(*queue==NULL)
		return 0;

	memset( *queue, 0, sizeof(shmQueue));
	//printf("*queue->head:%X *queue->last:%X\n", (*queue)->head, (*queue)->last);
	return 1; 
}

int queue_put(shmQueue *queue, struct QueueElement *element)
{
	if(queue==NULL || element==NULL)
	{
		//printf("NULL errror!queue pointer:%X element pointer:%X\n",queue,element);
		return 0;
	}
	if(queue->head == NULL && queue->last == NULL)
	{
		element->pre = NULL;
		element->next = NULL;
		queue->last = element;
		queue->head = element;
		queue->length++;
		return 1;
	}
	element->pre = NULL;
	queue->head->pre = element;
	element->next = queue->head;
	queue->head = element;
	//printf("event type:%d \n", ((event_t *)queue->head->element)->type);
	queue->length++;

	return 1;	
}

/*
	element return NULL that mean no element
*/
int queue_get(shmQueue *queue, struct QueueElement **element)
{
	if(queue->head == NULL && queue->last == NULL)
	{
		return 0;
	}
	*element = (struct QueueElement *)queue->last;

	queue->last = queue->last->pre;
	if(queue->last == NULL)
	{
		queue->head = NULL;
	}
	queue->length--;

	return 1;
}

int queue_rm(shmQueue *queue, struct QueueElement *element)
{
	struct QueueElement *pre, *next;

	if(queue->head == NULL && queue->last == NULL)
	{
		return 0;
	}

	if(queue->length==0)
	{
		return 0;
	}

	pre = element->pre;
	next = element->next;

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
		queue->head = NULL;
		queue->last = NULL;
	}
	else if(pre != NULL && next == NULL)/*element is last case*/
	{
		queue->last = pre;

		if(queue->last == NULL)
		{
			queue->head = NULL;
		}
	}
	else if(pre == NULL && next != NULL)/*element is head case*/
	{
		queue->head = next;

		if(queue->head == NULL)
		{
			queue->last = NULL;
		}
	}
	else if(pre != NULL && next != NULL)/*element is between last and head case*/
	{
		//nothing
		;
	}

	queue->length--;
	return 1;
}

int data_put(shmQueue *queue, void *data)
{
	struct QueueElement *element = NULL;
	
	element = dbgMem_malloc(sizeof(struct QueueElement));
	if(element==NULL) return 0;
	
	memset( element, 0, sizeof(struct QueueElement));
	
	element->data = data;
	
	return queue_put(queue, element);
}

int data_get(shmQueue *queue, void *data, unsigned int size)
{
	struct QueueElement *element = NULL;
	int rc = queue_get(queue, &element);
	
	if ( rc == 1)
	{
		memcpy( data, element->data, size);
		dbgMem_free(element->data);
		dbgMem_free(element);
	}

	return rc;
}

int data_first( shmQueue *queue, void *data, unsigned int size)
{
	queue->curr = queue->last;
	void *tmpData;
	
	if((queue->curr != NULL))
	{
		tmpData = queue->curr->data;
	}

	if((queue->curr != NULL) && (tmpData != NULL))
	{
		memcpy( data, tmpData, size);
		
		return 1;
	}
	
	return 0;
}

int data_next( shmQueue *queue, void *data, unsigned int size)
{
	queue->curr = queue->curr->pre;
	void *tmpData;
	
	if((queue->curr != NULL))
	{
		tmpData = queue->curr->data;
	}

	if((queue->curr != NULL) && (tmpData != NULL))
	{
		memcpy( data, tmpData, size);
		
		return 1;
	}
	
	return 0;
}

int data_rm( shmQueue *queue, void *data)
{

	if( data == queue->curr->data)
	{
		queue_rm( queue,  queue->curr);
		dbgMem_free( queue->curr->data);
		dbgMem_free( queue->curr);
		return 1;
	}

	return 0;
}

#define CHANGE_MEM_BASE( address, newBase, origBase) address?(void *)(((unsigned int)address - (unsigned int)origBase) + (unsigned int)newBase):NULL

//TODO change memory base address.
int data_first_dump( void *newBase, void *origBase, shmQueue *queue, void *data, unsigned int size)
{
	//queue->curr  = queue->last;
	queue->curr = CHANGE_MEM_BASE( queue->last, newBase, origBase);
	void *tmpData;
	
	if((queue->curr != NULL))
	{
		tmpData = CHANGE_MEM_BASE( queue->curr->data, newBase, origBase);
		//tmpData = queue->curr->data;
	}

	if((queue->curr != NULL) && (tmpData != NULL))
	{
		memcpy( data, tmpData, size);
		
		return 1;
	}
	
	return 0;
}

int data_next_dump( void *newBase, void *origBase, shmQueue *queue, void *data, unsigned int size)
{
	//queue->curr = queue->curr->pre;
	queue->curr = CHANGE_MEM_BASE( queue->curr->pre, newBase, origBase);
	void *tmpData;
	
	if((queue->curr != NULL))
	{
		//tmpData = queue->curr->data;
		tmpData =  CHANGE_MEM_BASE( queue->curr->data, newBase, origBase);
	}

	if((queue->curr != NULL) && (tmpData != NULL))
	{
		memcpy( data, tmpData, size);
		
		return 1;
	}
	
	return 0;
}

int data_rm_dump(void *newBase, void *origBase, shmQueue *queue, void *data)
{

	if(  CHANGE_MEM_BASE( data, newBase, origBase) == CHANGE_MEM_BASE( queue->curr->data, newBase, origBase))
	{
		queue_rm( queue,  CHANGE_MEM_BASE( queue->curr, newBase, origBase));
		dbgMem_free( CHANGE_MEM_BASE( queue->curr->data, newBase, origBase));
		dbgMem_free( CHANGE_MEM_BASE( queue->curr, newBase, origBase));
		return 1;
	}

	return 0;
}

