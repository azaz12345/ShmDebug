CC=gcc
CFLAG=-I./
LDFLAG=
OBJ= shmDebug.o shmList.o shmMalloc.o shmQueue.o

all: $(OBJ)

%.o:%.c
        $(CC) $< $(CFLAG) -c -o $@
