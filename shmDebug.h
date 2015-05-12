#ifndef __SHM_DEBUG_H_
#define __SHM_DEBUG_H_
#include <pthread.h>
#include "shmMalloc.h"
#include "shmQueue.h"


typedef struct debugMsg_s{
	unsigned int msgNo;
	char msg[256];
} debugMsg_t;

typedef struct dbgMemTrace_s{

	char funcName[64];
	unsigned int funcLine;
	void *address;
	
} dbgMemTrace_t;

typedef struct shm_debug_context_s{
	unsigned int msgCount;

	debugMsg_t debugmsg[500];

	pthread_mutexattr_t mutex_shared_attr;
	pthread_mutex_t traceMutex;
	shmQueue traceList;

	dbgMemPoolContext_t dbgMemPoolCtx;
	dbgMemPoolContext_t dbgMemPoolCtx_dump;
	unsigned char dbgMemoryPool[512*sizeof(dbgMemTrace_t)];
} shm_debug_context_t;

int initDebugShm();
int accessDebugShm();
int printDebugShm();
int pdcpShmlog( char *string, ...);

void shmDbgAddTrace(char *funcName, unsigned int funcLine, void *address);
void shmDbgRmoveTrace(void *address);
void shmDbgPrintTrace();

int printDebugShm();

#endif