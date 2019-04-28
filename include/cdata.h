/*
 * cdata.h: arquivo de inclusão de uso apenas na geração da libpithread
 *
 * VERSÃO: 29/03/2019
 *
 */
#ifndef __cdata__
#define __cdata__

// To work on macOS
#define _XOPEN_SOURCE 600

#if APPLE
    #include "sys/ucontext.h"
#else
    #include "ucontext.h"
#endif

/* Estados das threads
*/
#define	PROCST_APTO	0		/* Processo em estado apto a ser escalonado */
#define	PROCST_EXEC	1		/* Processo em estado de execução */
#define	PROCST_BLOQ	2		/* Processo em estado bloqueado */
#define	PROCST_TERMINO	3	/* Processo em estado de terminado */


// MARK: - Thread priority
//enum ThreadPriority {
//    High, Medium, Low
//};
#define NUMBER_PRIORITY_QUEUES 3
#define THREAD_PRIORITY_LOW 2
#define THREAD_PRIORITY_MEDIUM 1
#define THREAD_PRIORITY_HIGH 0

// MARK: - Thread State
//enum ThreadState {
//    Creation, Ready, Running, Blocked, Finish
//};
#define THREAD_STATE_CREATION 0
#define THREAD_STATE_READY 1
#define THREAD_STATE_RUNNING 2
#define THREAD_STATE_BLOCKED 3
#define THREAD_STATE_FINISH 4

typedef struct s_TCB { 
	/* OS CAMPOS ABAIXO NÃO PODEM SER ALTERADOS OU REMOVIDOS
	*/
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra (CONFORME DEFINES ANTERIORES)
	int 		prio;		// Prioridade associada a thread
	ucontext_t 	context;	// contexto de execução da thread (SP, PC, GPRs e recursos) 
	
	/* A PARTIR DESSE PONTO PODEM SER ACRESCENTADOS CAMPOS NECESSÁRIOS AO DESENVOLVIMENTO DA BIBLIOTECA
	*/
	
} TCB_t;

typedef struct s_join {
    TCB_t* blocked_thread;
    TCB_t* target_thread;
} join_t;

struct TCB_t* join_list[999];

#include "support.h"
// MARK: Queues
FILA2 ready[NUMBER_PRIORITY_QUEUES];
FILA2 blocked;
FILA2 joins;

#endif
