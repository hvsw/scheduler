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
TCB_t *tYield = NULL;

// MARK: - Local vars
TCB_t* runningThread;

/*!
 @brief Partiremos do 1 pois a 0 será a main
 */
int currentThreadId = 1;

int isThreadAtQueue(TCB_t *thread, PFILA2 queue) {
    FirstFila2(queue);
    TCB_t* t = NULL;
    do {
        t = GetAtIteratorFila2(queue);
        if (t == NULL) {
            return 0;
        } else {
            if (t->tid == thread->tid) {
                return 1;
            }
        }
    } while (NextFila2(queue) == 0);
    
    return 0;
    
}

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
    int prio = THREAD_PRIORITY_LOW;
    while (prio <= THREAD_PRIORITY_LOW) {
        PFILA2 queue = getQueueWithPriority(prio);
        FirstFila2(queue);
        TCB_t *t = NULL;
        printf("Queue %d::: ", prio);
        do {
            t = GetAtIteratorFila2(queue);
            printf("%d, ", t->tid);
        } while (t != NULL && NextFila2(queue) == 0);
        printf("\n");
        prio++;
    }
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
    schedulerContext.uc_link = 0; //&main_thread.context; //scheduler volta para main
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
                // Invalid pointer, checking the reason...
                if (debug) {
                    if (queue->first == NULL) {
//                        printf("Empty queue\n");
                        return NULL;
                    }
                    
                    if (queue->it == NULL) {
//                        printf("Invalid iterator\n");
                        return NULL;
                    }
                }
                
                // If didn't returned it means that it was just pointing to a NULL pointer, follow the baile to check if there's a next one.
                // Yeah, this should be a just in case thing, the support lib should handle this and add error handling/codes but...
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
        TCB_t *t = GetAtIteratorFila2(queue);
        if (debug) {
            if (t == NULL) {
                if (queue->first == NULL) {
                    printf("Empty queue\n");
                } else {
                    if (queue->it == NULL) {
                        printf("Invalid iterator\n");
                    }
                }
            }
            
//            printf("Error(%d) while trying to set iterator to first position\n", setIteratorToFirstResult);
//            printf("  first: %p\n", queue->first);
//            printf("  it: %p\n", queue->it);
//            printf("  last: %p\n", queue->last);
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
//        if (debug) {
//            printf("Searching queue(%d)\n", priority);
//        }
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
    FirstFila2(&blocked);
    
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
            print_all_queues2();
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
    if (debug) {
        printf("***** schedule() *****\n");
    }
    
    // Se running thread nula, quer dizer que foi yield.
    // Logo, se não nula devemos assumir que a thread encerrou
    if (runningThread != NULL && runningThread != &mainThread) {
        if (debug) {
            printf("Running thread(%d) NOT NULL, NOT MAIN and NOT YIELD; Assume thread finished; releasing threads from tid(%d)\n", runningThread->tid, runningThread->tid);
        }
        
        runningThread->state = THREAD_STATE_FINISH;
        releaseThreadsFromTid(runningThread->tid);
    } else {
        // YIELD
        // Só vai utilizar quando for chamado do yield
        if (tYield != NULL && runningThread != &mainThread) {
            // Thread que deu yield é a que está em execução
            runningThread = tYield;
        } else {
//            printf("Entrou no caso do yield mas tYield not NULL\n");
//            exit(999);
        }
    }
    
    TCB_t *nextThread = findNextThread();
    
    // Remove next thread da fila de aptos
    if (nextThread == NULL) {
        if (debug) {
            printf("No more threads to execute\n");
        }
    } else {
        
        if (debug) {
            printf("Removing next thread(%d) from queue(%d)\n", nextThread->tid, nextThread->prio);
        }
        PFILA2 nextThreadQueue = getQueueWithPriority(nextThread->prio);
        int moveIteratorToFirst = FirstFila2(nextThreadQueue);
        if (moveIteratorToFirst != 0) {
            if (debug) {
                printf("Error moving queue(%d) iterator to first! Reason: ", moveIteratorToFirst);
                if (nextThreadQueue->first == NULL) {
                    printf("empty queue");
                }
                
                if (nextThreadQueue->it == NULL) {
                    printf("invalid iterator");
                }
            }
            
            return;
        } else {
            if (debug) {
                printf("Moved queue(%d) iterator to first to delete thread (%d)\n", nextThread->prio, nextThread->tid);
            }
            
            int deleteResult = DeleteAtIteratorFila2(nextThreadQueue);
            if (debug) {
                if (deleteResult == 0) {
                    printf("Deleted at iterator!\n");
                    //                print_all_queues2();
                } else {
                    printf("DeleteAtIteratorFila2 queue(%d): %d", nextThread->prio, deleteResult);
                    switch (deleteResult) {
                        case -DELITER_VAZIA:
                            printf("Empty queue(%d)", nextThread->prio);
                            break;
                            
                        case -DELITER_INVAL:
                            printf("Invalid queue(%d) iterator", nextThread->prio);
                            break;
                    }
                    printf("\n");
                }
            }
        }
    }
    
    // Adiciona runningThread na fila de aptos
    PFILA2 runningThreadQueue = getQueueWithPriority(runningThread->prio);
    int first = FirstFila2(runningThreadQueue);
    if (first != 0) {
        if (debug) {
//            printf("WARNING: Maybe this queue(%d) is empty or we've got an error(%d)!\n", runningThread->prio, first);
        }
    }
    
    TCB_t *t = GetAtIteratorFila2(runningThreadQueue);
    if (t == NULL) {
        if (runningThreadQueue->first == NULL) {
            if (debug) {
                printf("Empty queue(%d), appending thread(%d)...\n", runningThread->prio, runningThread->tid);
            }
            
            if (!isThreadAtQueue(runningThread, runningThreadQueue)) {
                int appendNextThread = AppendFila2(runningThreadQueue, runningThread);
                if (appendNextThread == 0) {
                    if (debug) {
                        printf("Thread(%d) added to queue(%d)\n", runningThread->tid, runningThread->prio);
                        print_all_queues2();
                    }
                }
            }
            
        } else {
            if (runningThreadQueue->it == NULL) {
                if (debug) {
                    printf("Invalid iterator\n");
                }
            } else {
                int insert = InsertBeforeIteratorFila2(runningThreadQueue, runningThread);
                if (insert == 0) {
                    if (debug) {
                        printf("Inserted thread(%d) into queue(%d)\n", runningThread->tid, runningThread->prio);
                    }
                    print_all_queues2();
                } else {
                    if (debug) {
                        printf("Error(%d) inserting before iterator at queue(%d)\n", insert, runningThread->prio);
                    }
                }
            }
        }
    } else {
        if (!isThreadAtQueue(runningThread, runningThreadQueue)) {
            int appendNextThread = AppendFila2(runningThreadQueue, runningThread);
            if (appendNextThread == 0) {
                if (debug) {
                    printf("Thread(%d) added to queue(%d)\n", runningThread->tid, runningThread->prio);
                    print_all_queues2();
                }
            }
        }
    }
    runningThread->state = THREAD_STATE_READY;
    
    // Coloca em execucao a proxima
    if (debug) {
        printf("Setting thread(%d) as running thread!\n", nextThread->tid);
    }
    runningThread = nextThread;
    runningThread->state = THREAD_STATE_RUNNING;
    
    if (debug) {
        printf("Setting context now!\n");
    }
    
    setcontext(&runningThread->context);
}

int generateThreadId() {
    return currentThreadId++;
}

// MARK: - Compact thread functions
int ccreate (void* (*start)(void*), void *arg, int prio) {
    initIfNeeded();
    
    printf("***** ccreate(prio=%d) *****\n", prio);
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
        tcb->state = THREAD_STATE_CREATION;
        
        PFILA2 newThreadQueue = getQueueWithPriority(tcb->prio);
        
        int first = FirstFila2(newThreadQueue);
        
        if (first != 0) {
            if (debug) {
                printf("WARNING: Maybe this queue(%d) is empty or we've got an error(%d)!\n", tcb->prio, first);
            }
        }
        
        printf("Trying to add thread(%d) to queue(%d)\n", tcb->tid, tcb->prio);
        TCB_t *t = GetAtIteratorFila2(newThreadQueue);
        if (t == NULL) {
            if (newThreadQueue->first == NULL) {
                if (debug) {
                    printf("Empty queue(%d), appeding(%d)...\n", tcb->prio, tcb->tid);
                }
                
                if (!isThreadAtQueue(tcb, newThreadQueue)) {
                    int appendNextThread = AppendFila2(newThreadQueue, tcb);
                    if (appendNextThread == 0) {
                        if (debug) {
                            printf("Thread(%d) appended to queue(%d)\n", tcb->tid, tcb->prio);
                            print_all_queues2();
                        }
                    }
                }
                
            } else {
                if (newThreadQueue->it == NULL) {
                    if (debug) {
                        printf("Invalid iterator\n");
                    }
                } else {
                    if (debug) {
                        printf("Not empty queue(%d), inserting(%d)...\n", tcb->prio, tcb->tid);
                    }
                    
                    
                    int insert = InsertBeforeIteratorFila2(newThreadQueue, tcb);
                    if (insert == 0) {
                        if (debug) {
                            printf("Inserted thread(%d) into queue(%d)\n", tcb->tid, tcb->prio);
                        }
                        print_all_queues2();
                    } else {
                        if (debug) {
                            printf("Error(%d) inserting before iterator at queue(%d)\n", insert, tcb->prio);
                        }
                    }
                }
            }
        } else {
            if (!isThreadAtQueue(tcb, newThreadQueue)) {
                if (newThreadQueue->first == NULL) {
                    printf("Empty queue(%d)!\n", prio);
                }
                
                if (newThreadQueue->it == NULL) {
                    printf("Invalid queue(%d) iterator!\n", prio);
                }
                
                if (first != 0) {
                    if (debug) {
                        printf("WARNING: Maybe this queue(%d) is empty or we've got an error(%d)!\n", tcb->prio, first);
                    }
                }
                
                int appendNextThread = InsertBeforeIteratorFila2(newThreadQueue, tcb);
                if (appendNextThread == 0) {
                    if (debug) {
                        printf("Thread(%d) inserted to queue(%d)\n", tcb->tid, tcb->prio);
                    }
                } else {
                    if (debug) {
                        printf("Error inserting thread(%d) before iterator at queue(%d)\n", tcb->tid, tcb->prio);
                    }
                }
                print_all_queues2();
            }
        }
        
        swapcontext(&runningThread->context, &schedulerContext);
        return tcb->tid;
    } else {
        if (debug) {
            printf("Erro trying to getcontext\n");
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
    if (debug) {
        printf("***** cyield(%d) *****\n", runningThread->tid);
    }
    
    tYield = runningThread;
    runningThread = NULL;
    swapcontext(&tYield->context, &schedulerContext);
    return CYIELD_SUCCESS;
//    } else {
//        if (debug) {
//            printf("Erro adicionando thread(%d) a fila(%d)\n", runningThread->tid, runningThread->prio);
//        }
//        return CYIELD_ERROR;
//    }
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
//            printf("Looking for tid(%d) at priority queue(%d)\n", tid, prio);
        }
        
        tcb = findTcbWithIdAtPriorityQueue(tid, prio);
        if (tcb == NULL) {
            if (debug) {
//                printf("Thread(%d) not found at queue(%d)", tid, prio);
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
    print_all_queues2();
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
    print_all_queues2();
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
