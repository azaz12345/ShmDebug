#ifndef __SHM_MALLOC_H_
#define __SHM_MALLOC_H_

#define USED 1

typedef struct {
  unsigned int size;
} UNIT;

typedef struct {
  UNIT* free;
  UNIT* heap;
} dbgMemPoolContext_t;


void* dbgMem_malloc(unsigned int bytes);
void dbgMem_free(void *ptr);
void dbgMem_Init(dbgMemPoolContext_t *ctx, void *heap, unsigned int len );
void dbgMem_Compact( void );

#endif
