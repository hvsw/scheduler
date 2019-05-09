//
//  cthread2.c
//  cthread
//
//  Created by Henrique Valcanaia on 2019-05-8.
//  Copyright © 2019 Henrique Valcanaia. All rights reserved.
//

#define _XOPEN_SOURCE 600
#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"
#include <string.h>
#include <stdlib.h>
#include "debug.c"

#define CCREATE_ERROR -1
#define CYIELD_ERROR -1
#define MAIN_THREAD_ID 0
#define ERROR -1
#define NOT_IMPLEMENTED -9

#define FREE(x) do { if ((x) != NULL) {free(x); x = NULL;} } while(0)

// MARK: Functions prototypes
void schedule(void);

int debug = 1;

// MARK: - Local vars
TCB_t* runningThread;

/*!
 @brief Partiremos do 1 pois a 0 será a main
 */
int currentThreadId = 1;

// MARK: Utils
PFILA2 getQueueWithPriority(int prio) {
    if (prio > THREAD_PRIORITY_LOW || prio < THREAD_PRIORITY_HIGH) {
        if (debug) {
            printf("Invalid priority %d", prio);
        }
        return NULL;
    }
    return &ready[prio];
}

void print_all_queues2() {
    if (!debug) {
        return;
    }
    printf("***** PRINT ALL QUEUES | BEGIN *****\n");
    int prio = THREAD_PRIORITY_LOW;
    while (prio <= THREAD_PRIORITY_LOW) {
        PFILA2 queue = getQueueWithPriority(prio);
        FirstFila2(queue);
        TCB_t *t = NULL;
        printf("Queue %d\n", prio);
        do {
            t = GetAtIteratorFila2(queue);
            printf("%d, ", t->tid);
        } while (t != NULL && NextFila2(queue) == 0);
        printf("\n");
        prio++;
    }
    printf("***** PRINT ALL QUEUES | END *****\n");
}

// MARK: - Compact Thread Library

// MARK: Control variables
int initialized = 0;
TCB_t mainThread;
ucontext_t schedulerContext;

// MARK: Init
char ss_sp_scheduler[SIGSTKSZ];
void initScheduler() {
    getcontext(&schedulerContext);
    schedulerContext.uc_link = &(mainThread.context);
    schedulerContext.uc_stack.ss_sp = ss_sp_scheduler;
    schedulerContext.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&schedulerContext, (void (*)(void))schedule, 0);
}

void createMainTcb() {
    runningThread = &mainThread;
    
    mainThread.tid = MAIN_THREAD_ID;
    // SEÇÃO 4 DO PDF DO TRABALHO:
    // A thread main tem a menor prioridade (baixa).
    mainThread.prio = THREAD_PRIORITY_LOW;
    getcontext(&mainThread.context);
}

void initQueues() {
    // Ready queues
    int priority = THREAD_PRIORITY_HIGH; // 0
    while (priority < NUMBER_PRIORITY_QUEUES) {
        if (debug) {
            printf("Initializing queue (%d)\n", priority);
        }
        if (CreateFila2(&ready[priority]) == 0) {
            if (debug) {
                printf("Queue(%d) created!\n", priority);
            }
        } else {
            if (debug) {
                printf("Error creating ready queue with priority %d\n", priority);
            }
        }
        priority++;
    }
    
    // Blocked
    if (CreateFila2(&blocked) == 0) {
        if (debug) {
            printf("Blocked queue created!\n");
        }
    } else {
        if (debug) {
            printf("Error creating blocked queue\n");
        }
    }
    
    // Joins
    if (CreateFila2(&joins) == 0) {
        if (debug) {
            printf("Joins queue created!\n");
        }
    } else {
        if (debug) {
            printf("Error creating joins queue\n");
        }
    }
}

void initIfNeeded() {
    if (initialized == 0) {
        if (debug) {
            printf("Initialiazing cthread...\n");
        }
        initQueues();
        createMainTcb();
        initScheduler();
        initialized = 1;
        if (debug) {
            printf("cthread initialized!\n");
        }
    }
}

TCB_t* getFirstFromQueue(PFILA2 queue) {
    TCB_t* firstThread = NULL;
    
    int setIteratorToFirstResult = FirstFila2(queue);
    if (setIteratorToFirstResult == 0) {
        int nextResult = -NXTFILA_ITERINVAL;
        while (firstThread == NULL) {
            firstThread = (TCB_t*)GetAtIteratorFila2(queue);
            if (firstThread == NULL) {
                if (debug) {
                    if (queue->first == NULL) {
                        printf("Empty queue\n");
                        return NULL;
                    }
                    
                    if (queue->it == NULL) {
                        printf("Invalid iterator\n");
                        return NULL;
                    }
                }
                
            } else {
                if (debug) {
                    printf("Found next thread(%d)!\n", firstThread->tid);
                }
                
                break;
            }
            
            nextResult = NextFila2(queue);
            if (debug) {
                switch (nextResult) {
                    case -NXTFILA_VAZIA:
                        printf("Empty queue!\n");
                    case -NXTFILA_ITERINVAL:
                        printf("Invalid iterator!\n");
                    case -NXTFILA_ENDQUEUE:
                        printf("Queue end reached!\n");
                }
                break;
            }
        }
    } else {
        if (debug) {
            printf("Error(%d) while trying to set iterator to first position\n", setIteratorToFirstResult);
            printf("  first: %p\n", queue->first);
            printf("  it: %p\n", queue->it);
            printf("  last: %p\n", queue->last);
        }
    }
    
    return firstThread;
}

TCB_t* findNextThread() {
    if (debug) {
        printf("Looking for next thread...\n");
    }
    
    TCB_t* nextThread = NULL;
    int priority = THREAD_PRIORITY_HIGH;
    while (priority <= THREAD_PRIORITY_LOW) {
        if (debug) {
            printf("Searching queue(%d)\n", priority);
        }
        PFILA2 queue = getQueueWithPriority(priority);
        if (queue == NULL) {
            if (debug) {
                printf("Queue(%d) not found!\n", priority);
            }
            return NULL;
        }
        
        nextThread = getFirstFromQueue(queue);
        if (nextThread == NULL) {
            if (debug) {
                printf("No next thread for queue(%d)\n", priority);
            }
        } else {
            break;
        }
        
        priority++;
    }
    
    return nextThread;
}

void destroyJoin(join_t* join) {
    if(FirstFila2(&blocked) != 0) {
        if (debug) {
            printf("Fila de Aptos vazia ou ERRO\n");
        }
        return;
    }
    
    TCB_t* thread = NULL;
    do {
        thread = GetAtIteratorFila2(&blocked);
        if (thread == NULL) {
            if (debug) {
                if (blocked.it == NULL) {
                    printf("Empty blocked queue\n");
                }
                
                if (blocked.first == NULL) {
                    printf("Invalid iterator for blocked queue\n");
                }
            }
            return;
        }
        if (thread == join->blocked_thread) {
            thread->state = THREAD_STATE_READY;
            DeleteAtIteratorFila2(&blocked);
            AppendFila2(&ready[thread->prio], (void*) thread);
            return;
        }
    } while (NextFila2(&blocked) == 0);
}

void releaseThreadsFromTid(int tid) {
    if(FirstFila2(&joins) != 0) {
        if (debug) {
            printf("Empty join queue\n");
        }
        return;
    }
    
    join_t* join;
    do {
        join = GetAtIteratorFila2(&joins);
        if (join == NULL)
            return;
        if (join->target_thread->tid == tid) {
            destroyJoin(join);
            return;
        }
    } while (NextFila2(&joins) == 0);
}

void schedule() {
    // pega a proxima thread
    if (debug) {
        printf("***** schedule() *****\n");
    }
    
    // Se running thread nula, quer dizer que foi yield.
    // Logo, se não nula devemos assumir que a thread encerrou
    if (runningThread != NULL && runningThread != &mainThread) {
        if (debug) {
            printf("Running thread not NULL; Assume thread finished; releasing threads from tid(%d)\n", runningThread->tid);
        }
        
        releaseThreadsFromTid(runningThread->tid);
    }
    
    //  move thread atual pra apto
    if (debug) {
        printf("Moving running thread(%d) to ready(%d)...\n", runningThread->tid, runningThread->prio);
    }
    
    int moveRunningThreadToReady = AppendFila2(&ready[runningThread->prio], runningThread);
    
    if (moveRunningThreadToReady != 0) {
        if (debug) {
            printf("Error moving thread(%d) to queue(%d)\n", moveRunningThreadToReady, runningThread->prio);
        }
    }
    
    // remove da fila de aptos
    PFILA2 readyQueue = getQueueWithPriority(runningThread->prio);
    
    if (debug) {
        printf("Got queue(%d)\n", runningThread->prio);
    }
    
    int moveIteratorToFirst = FirstFila2(readyQueue);
    if (moveIteratorToFirst != 0) {
        if (debug) {
            printf("Unable to move iterator to first! FirstFila2 returned: %d\n", moveIteratorToFirst);
        }
    } else {
        if (debug) {
            printf("Moved to first\n");
        }
    }
    
    int deleteResult = DeleteAtIteratorFila2(readyQueue);
    if (deleteResult < 0) {
        if (debug) {
            printf("DeleteAtIteratorFila2 queue(%d): %d", runningThread->prio, deleteResult);
            switch (deleteResult) {
                case -DELITER_VAZIA:
                    printf("Empty queue");
                    break;
                    
                case -DELITER_INVAL:
                    printf("Invalid iterator");
                    break;
            }
            printf("\n");
        }
    } else {
        if (debug) {
            printf("Deleted at iterator!\n");
        }
    }

    TCB_t *nextThread = findNextThread();
    if (nextThread == NULL) {
        if (debug) {
            printf("No more threads to execute\n");
        }
    } else {
        // coloca em execucao a proxima
        if (debug) {
            printf("Setting thread(%d) as running thread!\n", nextThread->prio);
        }
        runningThread = nextThread;
        
        if (debug) {
            printf("Setting context!\n");
        }
        
        // Ao setar o contexto da thread do testes1/caso1 ocorre seg fault.
        // Temos que verificar onde parou o contexto da thread que deu problema e continuar debugando
        setcontext(&runningThread->context);
    }
}

int generateThreadId() {
    return currentThreadId++;
}

// MARK: - Compact thread functions
int ccreate (void* (*start)(void*), void *arg, int prio) {
    initIfNeeded();
    
    ucontext_t context;
    if (getcontext(&context) == 0) {
        char tcb_stack[SIGSTKSZ];
        context.uc_link = &schedulerContext;
        context.uc_stack.ss_sp = tcb_stack;
        context.uc_stack.ss_size = SIGSTKSZ;
        makecontext(&context, (void (*)(void)) start, 1, arg);
        
        TCB_t* tcb = (TCB_t*) malloc(sizeof(TCB_t));
        tcb->tid = generateThreadId();
        tcb->context = context;
        tcb->prio = prio;
        if (AppendFila2(&ready[tcb->prio], (void*) tcb) == 0) {
            print_all_queues2();
            if (debug) {
                printf("Thread(%d) added to queue(%d)\n", tcb->tid, tcb->prio);
            }
            
            if (runningThread == NULL || runningThread == &mainThread) {
                
                if (debug) {
                    if (runningThread == NULL) {
                        printf("ccreate with NULL thread");
                    }
                    
                    if (runningThread == &mainThread) {
                        printf("ccreate from main thread");
                    }
                    printf(", adding new thread(%d) to CPU\n", tcb->tid);
                }
                
                swapcontext(&(runningThread->context), &schedulerContext);
            }
            return tcb->tid;
        } else {
            if (debug) {
                printf("Erro ao adicionar(%d) na fila(%d)\n", tcb->tid, tcb->prio);
            }
            return CCREATE_ERROR;
        }
    } else {
        if (debug) {
            printf("Erro ao obter context\n");
        }
        return CCREATE_ERROR;
    }
}

/*!
 @brief Efetua cedencia voluntária de CPU
 @return Quando executada corretamente retorna CYIELD_SUCCESS (0), caso contrário, retorna CYIELD_ERROR (-1)
 */
#define CYIELD_SUCCESS 0
#define CYIELD_ERROR -1
int cyield(void) {
    printf("***** cyield(%d) *****\n", runningThread->tid);
    
    if (AppendFila2(&ready[runningThread->prio], (void*) runningThread) != 0) {
        if (debug) {
            printf("Erro adicionando thread(%d) a fila(%d)\n", runningThread->tid, runningThread->prio);
        }
        return CYIELD_ERROR;
    }
    
    swapcontext(&runningThread->context, &schedulerContext);
    
    return CYIELD_SUCCESS;
}

/*!
 @brief Altera a prioridade com id = tid
 @param tid identificador da thread cuja prioridade será alterada (deixar sempre esse campo como NULL em 2019/01)
 @param prio nova prioridade da thread
 @return Quando executada corretamente retorna CSETPRIO_SUCCESS (0), caso contrário, retorna CSETPRIO_ERROR (-1)
 */
#define CSETPRIO_SUCCESS 0
#define CSETPRIO_ERROR -1

int csetprio(int tid, int prio) {
    runningThread->prio = prio;
    return CSETPRIO_SUCCESS;
}

int isThreadTargeted(int tid) {
    if(FirstFila2(&joins) != 0) {
        if (debug) {
        printf("CJOIN: Fila de joins vazia ou ERRO\n");
        }
        return 1;
    }
    
    do {
        join_t* join = GetAtIteratorFila2(&joins);
        if (join == NULL)
            return 0;
        if (join->target_thread->tid == tid)
            return 1;
    } while (NextFila2(&joins) == 0);
    
    return 0;
}
/*!
 @brief Bloqueia a execução de uma thread aguardando o término da thread com id indicado no parametro 'tid'
 @discussion Exemplo para geração de um tid válido
 
 @code
 int id0, id1, i;
 id0 = ccreate(func0, (void *)&i);
 id1 = ccreate(func1, (void *)&i);
 
 printf("Eu sou a main após a criação de ID0 e ID1\n");
 
 cjoin(id0);
 cjoin(id1);
 
 printf("Eu sou a main voltando para terminar o programa\n");
 @endcode
 
 
 @param tid identificador da thread cujo término está sendo aguardado. Caso seja informado um tid inválido a função retornará um valor negativo indicando erro.
 @return Quando executada corretamente retorna CJOIN_SUCCESS (0 zero). Caso contrário, retorna CJOIN_ERROR (um valor negativo)
 */

TCB_t *findTcbWithIdAtPriorityQueue(int tid, int priority) {
    PFILA2 queue = getQueueWithPriority(priority);
    if (debug) {
        printf("Checking queue(%d)...\n", priority);
    }
    int moveIteratorToFirst = FirstFila2(queue);
    if (moveIteratorToFirst != 0) {
        if (debug) {
            printf("Unable to move iterator to first! FirstFila2 returned: %d\n", moveIteratorToFirst);
        }
        return NULL;
    }
    
    int next = NXTFILA_ITERINVAL;
    TCB_t* tcb;
    do {
        tcb = (TCB_t*) GetAtIteratorFila2(queue);
        if (tcb == NULL) {
            if (debug) {
                printf("NULL at iterator\n");
            }
            return NULL;
        }
        
        if (tcb->tid == tid) {
            if (debug) {
                printf("Thread(%d) found\n", tid);
            }
            return tcb;
        } else {
            if (debug) {
                printf("Not this thread(%d), next...\n", tcb->tid);
            }
        }
        
        next = NextFila2(queue);
        if (debug) {
            switch (next) {
                case -NXTFILA_VAZIA:
                    printf("Empty queue!\n");
                case -NXTFILA_ITERINVAL:
                    printf("Invalid iterator!\n");
                case -NXTFILA_ENDQUEUE:
                    printf("Queue end reached!\n");
            }
        }
    } while (next == 0);
    
    return NULL;
}

TCB_t* getTcbWithId(int tid) {
    // Find TCB for tid
    TCB_t *tcb = NULL;
    int prio = THREAD_PRIORITY_HIGH;
    while (prio <= THREAD_PRIORITY_LOW) {
        if (debug){
            printf("Looking for tid(%d) at priority queue(%d)\n", tid, prio);
        }
        
        tcb = findTcbWithIdAtPriorityQueue(tid, prio);
        if (tcb == NULL) {
            if (debug) {
                printf("Thread(%d) not found at queue(%d)", tid, prio);
            }
        } else {
            printf("Found thread(%d)\n", tcb->tid);
            break;
        }
        
        prio++;
    }
    
    return tcb;
}

#define CJOIN_SUCCESS 0
#define CJOIN_ERROR_THREAD_FINISHED -1
#define CJOIN_ERROR_THREAD_ALREADY_JOINED -2
#define CJOIN_ERROR_THREAD_NOT_FOUND -3
#define CJOIN_ERROR_CANT_JOIN_MAIN_THREAD -4
#define CJOIN_ERROR_APPEND_JOIN -5
int join(TCB_t* blockedThread, TCB_t* targetThread) {
    join_t* join = (join_t*) malloc(sizeof(join_t));
    join->blocked_thread = blockedThread;
    join->target_thread = targetThread;
    
    if (AppendFila2(&joins, (void*) join) != 0) {
        if (debug) {
            printf("Unable to append join(%d->%d) to queue\n", blockedThread->tid, targetThread->tid);
        }
        return CJOIN_ERROR_APPEND_JOIN;
    }
    
    return CJOIN_SUCCESS;
}

int cjoin(int tid) {
    if(tid == MAIN_THREAD_ID) {
        if (debug) {
            printf("Can't join main thread! \n");
        }
        return CJOIN_ERROR_CANT_JOIN_MAIN_THREAD;
    }
    
    if (isThreadTargeted(tid)) {
        if (debug) {
            printf("Thread(%d) already targeted\n", tid);
        }
        return CJOIN_ERROR_THREAD_ALREADY_JOINED;
    }
    
    
    TCB_t *targetThread = getTcbWithId(tid);
    if (targetThread == NULL) {
        if (debug) {
            printf("Thread(%d) not found!\n", tid);
        }
        
        return CJOIN_ERROR_THREAD_NOT_FOUND;
    }
    
    int joinResult = join(runningThread, targetThread);
    if (joinResult != CJOIN_SUCCESS) {
        return joinResult;
    }
    
    // Block running thread
    TCB_t* callerThread = runningThread;
    if(AppendFila2(&blocked, (void*)callerThread) != 0) {
        if (debug) {
            printf("Error moving running_thread(%d) to blocked queue\n", callerThread->tid);
        }
    }
    
    runningThread = NULL;
    
    swapcontext(&callerThread->context, &schedulerContext);
    
    return CJOIN_SUCCESS;
}



/*!
 @param sem ponteiro para uma variável do tipo csem_t. Aponta para uma estrutura de dados que representa a variável semáforo.
 @param count valor a ser usado na inicialização do semáforo. Representa a quantidade de recursos controlador pelo semáforo.
 @return Quando executada corretamente: retorna 0 (zero) Caso contrário, retorna um valor negativo.
 */
int csem_init(csem_t *sem, int count) {
    return NOT_IMPLEMENTED;
}

/*!
 @param sem ponteiro para uma variável do tipo semáforo.
 @return Quando executada corretamente retorna CWAIT_SUCCESS (0 zero), caso contrário, retorna CWAIT_ERROR (um valor negativo)
 */
int cwait(csem_t *sem) {
    return NOT_IMPLEMENTED;
}

/*!
 @param sem ponteiro para uma variável do tipo semáforo.
 @return Quando executada corretamente retorna CSIGNAL_SUCCESS (0 zero), caso contrário, retorna CSIGNAL_ERROR (um valor negativo)
 */
int csignal(csem_t *sem) {
    return NOT_IMPLEMENTED;
}

/*!
 @param name ponteiro para uma área de memória onde deve ser escrito um string que contém os nomes dos componentes do grupo e seus números de cartão. Deve ser uma linha por componente.
 @param size quantidade máxima de caracteres que podem ser copiados para o string de identificação dos componentes do grupo.
 @return Quando executada corretamente retorna CIDENTIFY_SUCCESS (0 zero), caso contrário, retorna CIDENTIFY_ERROR(um valor negativo)
 */
#define CIDENTIFY_SUCCESS 0
#define CIDENTIFY_ERROR -1
int cidentify (char *name, int size) {
    char * result = strncpy(name, "Henrique Valcanaia 240501\nLucas Bauer - 237054\nAlvaro Souza Pereira da Silva - 231162\n", size);
    return result == NULL ? CIDENTIFY_ERROR : CIDENTIFY_SUCCESS;
}
