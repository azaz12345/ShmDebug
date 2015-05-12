

#if 1
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shmMalloc.h"

dbgMemPoolContext_t *dbgMemPoolCtx = NULL;
dbgMemPoolContext_t *dbgMemPoolCtx_dump = NULL;


static UNIT* compact( UNIT *p, unsigned int nsize )
{
       unsigned int bsize, psize;
       UNIT *best;

       best = p;
       bsize = 0;

       while( psize = p->size, psize )
       {
              if( psize & USED )
              {
                  if( bsize != 0 )
                  {
                      best->size = bsize;
                      if( bsize >= nsize )
                      {
                          return best;
                      }
                  }
                  bsize = 0;
                  best = p = (UNIT *)( (void *)p + (psize & ~USED) );
              }
              else
              {
                  bsize += psize;
                  p = (UNIT *)( (void *)p + psize );
              }
       }

       if( bsize != 0 )
       {
           best->size = bsize;
           if( bsize >= nsize )
           {
               return best;
           }
       }

       return 0;
}

void* dbgMem_malloc(unsigned int bytes)
{
	unsigned int fsize;
	UNIT *p;

	if( bytes == 0 ) return 0;

	bytes  += 3 + sizeof(UNIT);
	bytes >>= 2;
	bytes <<= 2;

	if( dbgMemPoolCtx->free == 0 || bytes > dbgMemPoolCtx->free->size )
	{
		dbgMemPoolCtx->free = compact( dbgMemPoolCtx->heap, bytes );
		if( dbgMemPoolCtx->free == 0 ) return 0;
	}

	p = dbgMemPoolCtx->free;
	fsize = dbgMemPoolCtx->free->size;

	if( fsize >= bytes + sizeof(UNIT) )
	{
		dbgMemPoolCtx->free = (UNIT *)( (unsigned int)p + bytes );
		dbgMemPoolCtx->free->size = fsize - bytes;
	}
	else
	{
		dbgMemPoolCtx->free = 0;
		bytes = fsize;
	}

	p->size = bytes | USED;

	return (void *)( (void *)p + sizeof(UNIT) );
}

void dbgMem_free(void *ptr)
{
	 if( ptr )
	 {
		 UNIT *p;

		 p = (UNIT *)( (void *)ptr - sizeof(UNIT) );
		 p->size &= ~USED;
	 }
}

void dbgMem_Init(dbgMemPoolContext_t *ctx, void *heap, unsigned int len )
{
	dbgMemPoolCtx = ctx;
	 len >>= 2;
	 len <<= 2;
	 dbgMemPoolCtx->free = dbgMemPoolCtx->heap = (UNIT *) heap;
	 dbgMemPoolCtx->free->size = dbgMemPoolCtx->heap->size = len - sizeof(UNIT);
	 *(unsigned int *)((char *)heap + len - sizeof(UNIT)) = 0;
}


void dbgMem_Init_dump(dbgMemPoolContext_t *ctx_dump, void *heap_dump, unsigned int len)
{
	dbgMemPoolCtx_dump = ctx_dump;
	dbgMemPoolCtx_dump->heap = (UNIT *) heap_dump;
}


void dbgMem_Compact( void )
{
	 dbgMemPoolCtx->free = compact( dbgMemPoolCtx->heap, 0xffff );
}
#endif

