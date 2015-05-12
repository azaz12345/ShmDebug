#ifndef __SHM_QUEUE_H_
#define __SHM_QUEUE_H_

struct QueueElement{
	struct QueueElement *next, *pre;
	void *data;
};

typedef struct Queue_s{
	struct QueueElement *head;
	struct QueueElement *last;
	struct QueueElement *curr;
	long int length;
} shmQueue;

int queue_create(shmQueue **queue);
int queue_put(shmQueue *queue, struct QueueElement *element);
int queue_get(shmQueue *queue, struct QueueElement **element);

int data_put(shmQueue *queue, void *data);
int data_get(shmQueue *queue, void *data, unsigned int size);

int data_first( shmQueue *queue, void *data, unsigned int size);
int data_next( shmQueue *queue, void *data, unsigned int size);
int data_rm(shmQueue *queue, void *data);

int data_first_dump( void *newBase, void *origBase, shmQueue *queue, void *data, unsigned int size);
int data_next_dump( void *newBase, void *origBase, shmQueue *queue, void *data, unsigned int size);
int data_rm_dump(void *newBase, void *origBase, shmQueue *queue, void *data);

#endif