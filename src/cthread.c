
#define _XOPEN_SOURCE 600
#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"
#include <string.h>
#include <stdlib.h>
#include "debug.h"


#define ERROR -1
#define NOT_IMPLEMENTED -9

// MARK: - To make it work on mac
#ifdef _APPLE_
//    pthread_threadid_np(thread, &tcb.id)
#else
#ifdef unix

#endif
#endif


// MARK: - Local vars
TCB_t* runningThread;

// MARK: Queues
FILA2 ready;
FILA2 blocked;
FILA2 joins;

/*!
 @brief Partiremos do 1 pois a 0 será a main
 */
int currentThreadId = 1;

int ccreate (void* (*start)(void*), void *arg, int prio) {
	return NOT_IMPLEMENTED;
}

int csetprio(int tid, int prio) {
	return NOT_IMPLEMENTED;
}

int cyield(void) {
	return NOT_IMPLEMENTED;
}

int cjoin(int tid) {
	return NOT_IMPLEMENTED;
}

int csem_init(csem_t *sem, int count) {
	return NOT_IMPLEMENTED;
}

int cwait(csem_t *sem) {
	return NOT_IMPLEMENTED;
}

int csignal(csem_t *sem) {
	return NOT_IMPLEMENTED;
}

int cidentify(char *name, int size) {
    return sizeof(strncpy(name, "Henrique Valcanaia 240501\nLucas Bauer - 237054\nAlvaro Souza Pereira da Silva - 231162", size));
}


