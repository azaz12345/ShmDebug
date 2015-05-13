#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "shmDebug.h"
#include "shmMalloc.h"

shm_debug_context_t* shm = NULL;

int initDebugShm()
{
    int shmid;
    key_t key;
	//void **ppshm = &shm;

    /*
     * We'll name our shared memory segment
     * "5678".
     */
    key = 5566123;

    /*
     * Create the segment.
     */
    if ((shmid = shmget(key, sizeof(shm_debug_context_t), IPC_CREAT | 0666)) < 0) {
		return -1;
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (void *) -1) {
		return -2;
    }


	memset( shm, 0, sizeof(shm_debug_context_t));
	pthread_mutexattr_init(&shm->mutex_shared_attr);
	pthread_mutexattr_setpshared(&shm->mutex_shared_attr,PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&(shm->traceMutex),&shm->mutex_shared_attr); 
	dbgMem_Init( &shm->dbgMemPoolCtx, &shm->dbgMemoryPool[0], sizeof(shm->dbgMemoryPool) );

	return 1;
}

int accessDebugShm()
{
    int shmid;
    key_t key;

    /*
     * We'll name our shared memory segment
     * "5678".
     */
    key = 5566123;

    /*
     * Create the segment.
     */
    if ((shmid = shmget(key, sizeof(shm_debug_context_t), 0666)) < 0) {
		perror("shmget");
		return -1;
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (shm_debug_context_t *) -1) {
		perror("#shmat#");
		return -2;
    }

	dbgMem_Init_dump(&shm->dbgMemPoolCtx_dump, &shm->dbgMemoryPool[0], sizeof(shm->dbgMemoryPool));
	
	return 1;
}


static int _pdcpShmlog(shm_debug_context_t *shm_ctx, char *string, va_list argp)
{
	const char *p;

	int i;
	int ui;
	char *s;
	char fmtbuf[516] = {0}; 
	int maxBuflen = 512;
	int bufThreshold = 256;
	char *pBuf = NULL;
	void* pMalloc_original = NULL;
	void* pMalloc_new = NULL;

	int idx = 0;
	int breakloop = 0;
	int n = 0;

	pBuf = &fmtbuf[0];
	

	for(p = string; (*p != '\0')&&(breakloop == 0); p++)
	{
		//to support very long buf length
		if(idx >= bufThreshold)
		{
			maxBuflen = maxBuflen *2;
			bufThreshold = bufThreshold *2;
			pMalloc_new=malloc(maxBuflen + 4);

			if(pMalloc_new == NULL)
			{
				//no memory
				break; //break for loop;
			}
			
			if(pMalloc_original == NULL)
			{
				memcpy(pMalloc_new, &fmtbuf[0], idx);
			}
			else
			{
				memcpy(pMalloc_new, pMalloc_original, idx);
				free(pMalloc_original);
			}

			pMalloc_original = pMalloc_new;
			pMalloc_new = NULL;
			pBuf=(char*)pMalloc_original + idx;
		}
	
		if(*p != '%')
		{
			*pBuf =*p;
			pBuf++;
			idx++;
			continue;
		}

		switch(*++p)
		{
			case 'c':
				i = va_arg(argp, int);
				*pBuf =(char)i;
				pBuf++;
				idx++;
				break;

			case 'd':
				i = va_arg(argp, int);
				n= sprintf(pBuf,"%d",i);
				idx += n;
				pBuf = pBuf + n;
				break;

			case 'u':
				ui = va_arg(argp, unsigned int);
				n= sprintf(pBuf,"%u",ui);
				idx += n;
				pBuf = pBuf + n;
				break;

			case 's':
				s = va_arg(argp, char *);
				n = snprintf(pBuf,bufThreshold,"%s",s);
				idx += n;
				pBuf = pBuf + n;
				break;

			case 'x':
				i = va_arg(argp, int);
				n = sprintf(pBuf,"%x",i);
				idx += n;
				pBuf = pBuf + n;
				break;

			case '%':
				*pBuf='%';
				idx++;
				pBuf++;
				break;

			case '\0':
				*pBuf='%';
				idx++;
				pBuf++;
				
				breakloop = 1; //break loop
				break;

			default:
				*pBuf='%';
				idx++;
				pBuf++;
				
				i = va_arg(argp, int);
				*pBuf=(char)i;
				idx++;
				pBuf++;
				
				break;
		}

	}

	*pBuf='\0';

	memcpy(&shm_ctx->debugmsg[shm_ctx->msgCount%500].msg[0],fmtbuf, 256);
	shm_ctx->debugmsg[shm_ctx->msgCount%500].msg[255] = '\0';
	shm_ctx->debugmsg[shm_ctx->msgCount%500].msgNo = shm_ctx->msgCount;

	shm_ctx->msgCount++;
	return 1;
}

int pdcpShmlog( char *string, ...)
{
	int result = 0;
	static int isInitShm = 0;
	va_list ap;

	if(shm==NULL)
	{
		if(isInitShm==0)
		{
			if(initDebugShm(&shm)==1)
			{
				isInitShm = 1;
			}
			else
			{
				;
			}
		}
	}
	if(shm!=NULL){
		va_start(ap, string);

		result = _pdcpShmlog( shm, string, ap);

		va_end(ap);
	}
	else
	{
		result = -1;
	}

	return result;
}

/*
	Name: shmDbgAddTrace
	
	Description:
	This function is used to debug leakage issue, it can write in faked malloc function and record function name and address of malloc.
*/
void shmDbgAddTrace(char *funcName, unsigned int funcLine, void *address)
{

	pthread_mutex_lock (&(shm->traceMutex));

	if(address != NULL)
	{
		dbgMemTrace_t *traceData = dbgMem_malloc(sizeof(dbgMemTrace_t));

		if(traceData!=NULL)
		{
			traceData->address = address;
			traceData->funcLine = funcLine;
			memcpy(traceData->funcName, funcName, sizeof(traceData->funcName));
			traceData->funcName[(sizeof(traceData->funcName)/sizeof(traceData->funcName[0])) - 1] = '\0';

			//pdcpShmlog("[allocate][address = %x] [traceData' address = %x]\n", address, traceData);

			data_put(&shm->traceList, traceData);
		}
	}
	
	pthread_mutex_unlock (&(shm->traceMutex));
}

/*
	Name: shmDbgRmoveTrace
	
	Description:
	This function is used to debug leakage issue, it can write in faked free function.
*/
void shmDbgRmoveTrace(void *address)
{

	pthread_mutex_lock (&(shm->traceMutex));
	
	if(address!=NULL)
	{
		dbgMemTrace_t tmpTraceData = {{0},0,0};
		data_first(&shm->traceList, &tmpTraceData, sizeof(tmpTraceData));
		do{
			if(shm->traceList.curr == NULL) break;
			if(shm->traceList.curr->data == NULL) break;
			
			if(tmpTraceData.address!=address) continue;
	
			data_rm(&shm->traceList, shm->traceList.curr->data);
			dbgMem_free(shm->traceList.curr->data);
			//pdcpShmlog("[free][address = %x] [traceData' address = %x]\n", address, shm->traceList.curr->data);
			break;
				
		}while(data_next(&shm->traceList, &tmpTraceData, sizeof(tmpTraceData)));
	}
	
	pthread_mutex_unlock (&(shm->traceMutex));
}

#define CHANGE_MEM_BASE( address, newBase, origBase) address?(void *)(((unsigned int)address - (unsigned int)origBase) + (unsigned int)newBase):NULL

/*
	Description:
	This function cat print trace list on other process.
*/
void shmDbgPrintTrace()
{
	pthread_mutex_lock (&(shm->traceMutex));
	
	if(shm->traceList.last==NULL&&shm->traceList.head==NULL)
	{
		printf("trace list is empty!!\n");
		pthread_mutex_unlock (&(shm->traceMutex));
		return;
	}
	else
	{
		printf("unknown error!!\n");
		pthread_mutex_unlock (&(shm->traceMutex));
		return;
	}
	
	
	printf("[##traceList##][orig.heap = %x new.heap = %x]\n", shm->dbgMemPoolCtx.heap, shm->dbgMemPoolCtx_dump.heap);
	printf("[##traceList##][queue.last = %x queue.head = %x queue.len = %d]\n", shm->traceList.last, shm->traceList.head, shm->traceList.length);
	printf("[##traceList##][orig.last = %x new.last = %x]\n", shm->traceList.last, CHANGE_MEM_BASE( shm->traceList.last, shm->dbgMemPoolCtx_dump.heap, shm->dbgMemPoolCtx.heap));
	
	
	dbgMemTrace_t tmpTraceData = {{0},0,0};
	data_first_dump( shm->dbgMemPoolCtx_dump.heap, shm->dbgMemPoolCtx.heap, &shm->traceList, &tmpTraceData, sizeof(tmpTraceData));
	do{
		if(shm->traceList.curr == NULL) break;
		if(shm->traceList.curr->data == NULL) break;

		printf("[traceList][func = %s Line = %d][address = %x]\n", tmpTraceData.funcName, tmpTraceData.funcLine, tmpTraceData.address);
	}while(data_next_dump( shm->dbgMemPoolCtx_dump.heap, shm->dbgMemPoolCtx.heap, &shm->traceList, &tmpTraceData, sizeof(tmpTraceData)));
	
	pthread_mutex_unlock (&(shm->traceMutex));
}

int printDebugShm()
{
	int msgIndex = 0;

	for( msgIndex = 0; msgIndex < 500; msgIndex++)
	{
			printf("[MSG][%u] %s\n", shm->debugmsg[msgIndex].msgNo, &shm->debugmsg[msgIndex].msg[0]);
	}
	printf("---------------------------------------------\n");

	shmDbgPrintTrace();

	return 1;
}
