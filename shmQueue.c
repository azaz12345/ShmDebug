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

int queue_put(shmQueue *queue, QueueElement *element)
{
	return list_insert(queue, element);
}

/*
	element return NULL that mean no element
*/
int queue_get(shmQueue *queue, QueueElement **element)
{
	return list_popLast(queue, element);
}

int queue_rm(shmQueue *queue, QueueElement *element)
{
	return list_rm(queue, element);
}

int data_put(shmQueue *queue, void *data)
{
	QueueElement *element = NULL;
	
	element = dbgMem_malloc(sizeof(QueueElement));
	if(element==NULL) return 0;
	
	memset( element, 0, sizeof(QueueElement));
	
	element->data = data;
	
	return queue_put(queue, element);
}

int data_get(shmQueue *queue, void *data, unsigned int size)
{
	QueueElement *element = NULL;
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
		dbgMem_free( CHANGE_MEM_BASE( queue->curr, newBase, origBase));
		return 1;
	}

	return 0;
}

