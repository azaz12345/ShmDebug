#ifndef __SHM_LIST_H_
#define __SHM_LIST_H_

typedef struct listNode{
	struct listNode *next, *pre;
	void *data;
} listNode;


typedef struct list_s{
	listNode *head;
	listNode *last;
	listNode *curr;
	long int length;
} list_t;

int list_init(list_t *list);
int list_insert(list_t *list, listNode *node);
int list_rm(list_t *list, listNode *node);
int list_popLast(list_t *list, listNode **node);
int list_popHead(list_t *list, listNode **node);



#endif
