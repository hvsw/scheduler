
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

// MARK: - To make it work on mac
#ifdef _APPLE_
//    pthread_threadid_np(thread, &tcb.id)
#else
#ifdef unix

#endif
#endif

#define FREE(x) do { if ((x) != NULL) {free(x); x = NULL;} } while(0)

// MARK: - Local vars
TCB_t* running_thread;

/*!
 @brief Partiremos do 1 pois a 0 será a main
 */
int currentThreadId = 1;

// MARK: - Compact Thread Library

// MARK: Control variables
int initialized = 0;
ucontext_t scheduler_context;
TCB_t main_thread;

// MARK: Private helpers
void destroy_join(join_t* join) {
    //printf("vai destruir join entre %d e %d\n",join->blocked_thread->tid, join->target_thread->tid);
    if(FirstFila2(&blocked) != 0) {
        printf("Fila de Aptos vazia ou ERRO\n");
        return;
    }
    
    TCB_t* thread;
    do {
        thread = GetAtIteratorFila2(&blocked);
        if (thread == NULL)
            return;
        if (thread == join->blocked_thread) {
            thread->state = THREAD_STATE_READY;
            DeleteAtIteratorFila2(&blocked);
            AppendFila2(&ready[thread->prio], (void*) thread);
            return;
        }
    } while (NextFila2(&blocked) == 0);
}

void release_threads_from_tid(int tid) {
    if(FirstFila2(&joins) != 0) {
        printf("SCHD: Fila de Joins vazia ou ERRO\n");
        return;
    }
    
    join_t* join;
    do {
        join = GetAtIteratorFila2(&joins);
        if (join == NULL)
            return;
        if (join->target_thread->tid == tid) {
            destroy_join(join);
            return;
        }
    } while (NextFila2(&joins) == 0);
}

TCB_t* find_next_thread() {
    TCB_t* thread;
    int priority = THREAD_PRIORITY_HIGH;
    for (; priority <= THREAD_PRIORITY_LOW; priority++) {
        FILA2 queue = ready[priority];
        int set_iterator = FirstFila2(&queue);
        if (set_iterator == 0) {
            TCB_t* thread = (TCB_t*)GetAtIteratorFila2(&queue);
            if (thread != NULL) {
                printf("Found next thread!\n");
                return thread;
            }
        } else {
            printf("Error while trying to set iterator to first position\n");
        }
    }
    
    printf("Next thread not found, bummer!\n");
    return NULL;
}

#define REMOVE_THREAD_SUCCESS 0
#define REMOVE_THREAD_ERROR_OR_EMPTY_QUEUE -1
#define REMOVE_THREAD_TID_NOT_FOUND -2
int remove_thread(int tid, FILA2 queue) {
    if(FirstFila2(&queue) != 0) {
        printf("Fila (%d) vazia ou ERRO\n", queue);
        return REMOVE_THREAD_ERROR_OR_EMPTY_QUEUE;
    }
    
    TCB_t* thread;
    do {
        thread = GetAtIteratorFila2(&queue);
        if (thread == NULL) {
            printf("tid(%d) not found\n", tid);
            return REMOVE_THREAD_TID_NOT_FOUND;
        }
        
        if (thread->tid == tid) {
            printf("REMOVTHREAD: target = %d, found = %d\n", tid, thread->tid );
            int success = DeleteAtIteratorFila2(&queue);
            printf("Thread(%d) delete: %d\n", tid, success);
            return REMOVE_THREAD_SUCCESS;
        }
    } while (NextFila2(&queue) == 0);
    
    return REMOVE_THREAD_TID_NOT_FOUND;
}

void schedule() {
    printf("***************** schedule() *****************\n");

    // Se running thread nula, quer dizer que foi yield.
    // Logo, se não nula devemos assumir que a thread encerrou
    if (running_thread != NULL) {
        printf("Running thread not NULL; Assume thread finished; releasing threads from tid(%d)\n", running_thread->tid);
        release_threads_from_tid(running_thread->tid);
        running_thread = NULL;
    }
    
    TCB_t* next_thread = find_next_thread();
    printf("next_thread: tid(%d)\n", next_thread->tid);
    
    running_thread = next_thread;
    if (remove_thread(next_thread->tid, ready[next_thread->prio]) != 0) {
        printf("problema ao deletar thread corrente da fila de aptos\n");
    }
    
    running_thread->state = THREAD_STATE_RUNNING;
//    print_queue(ready[THREAD_PRIORITY_HIGH]);
    
    printf("Running thread: %d\n", running_thread->tid);
    
    setcontext(&running_thread->context);
}

int generate_thread_id() {
    return currentThreadId++;
}

// MARK: Init
char ss_sp_scheduler[SIGSTKSZ];
void init_scheduler() {
    getcontext(&scheduler_context);
    scheduler_context.uc_link = &(main_thread.context);
    scheduler_context.uc_stack.ss_sp = ss_sp_scheduler;
    scheduler_context.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&scheduler_context, (void (*)(void))schedule, 0);
}

void create_main_tcb() {
    running_thread = &main_thread;
    
    main_thread.tid = MAIN_THREAD_ID;
    main_thread.state = THREAD_STATE_RUNNING;
    getcontext(&main_thread.context);
}

void init_queues() {
    // Ready queues
    int priority = THREAD_PRIORITY_HIGH; // 0
    while (priority < NUMBER_PRIORITY_QUEUES) {
        printf("Initializing queue (%d)\n", priority);
        if (CreateFila2(&ready[priority]) == 0) {
            printf("Queue(%d) created!\n", priority);
        } else {
            printf("Error creating ready queue with priority %d\n", priority);
        }
        priority++;
    }
    
    // Blocked
    if (CreateFila2(&blocked) == 0) {
        printf("Blocked queue created!\n", priority);
    } else {
        printf("Error creating blocked queue\n");
    }
    
    // Joins
    if (CreateFila2(&joins) == 0) {
        printf("Joins queue created!\n", priority);
    } else {
        printf("Error creating joins queue\n");
    }
}

void initIfNeeded() {
    if (initialized == 0) {
        init_queues();
        create_main_tcb();
        init_scheduler();
        initialized = 1;
    }
}

// MARK: Public methods
int ccreate (void* (*start)(void*), void *arg, int prio) {
    initIfNeeded();
    
    ucontext_t context;
    if (getcontext(&context) == 0) {
        char tcb_stack[SIGSTKSZ];
        context.uc_link = &scheduler_context;
        context.uc_stack.ss_sp = tcb_stack;
        context.uc_stack.ss_size = SIGSTKSZ;
        makecontext(&context, (void (*)(void)) start, 1, arg);
        
        TCB_t* tcb = (TCB_t*) malloc(sizeof(TCB_t));
        tcb->state = THREAD_STATE_CREATION;
        tcb->tid = generate_thread_id();
        tcb->context = context;
        tcb->prio = prio;
        if (AppendFila2(&ready[prio], (void*) tcb) == 0) {
            printf("Adicionou tid(%d) na fila\n", tcb->tid);
            return tcb->tid;
        } else {
            printf("Erro ao adicionar(%d) na fila\n");
            return CCREATE_ERROR;
        }
    } else {
        printf("Erro ao obter context\n");
        return CCREATE_ERROR;
    }
}

/*!
 @brief Retorna um TCB na fila "queue" com o thread id "tid"
 */
TCB_t* find_thread_with_id(int tid, PFILA2 queue) {
    int moveIteratorToFirst = FirstFila2(queue);
    if (moveIteratorToFirst != 0) {
        printf("Unable to move iterator to first! FirstFila2 returned: %d\n", moveIteratorToFirst);
        return NULL;
    }
    
    int next = NXTFILA_ITERINVAL;
    TCB_t* thread;
    do {
        /*
         Função:    Retorna o conteúdo do nodo endereçado pelo iterador da lista "pFila"
         Ret:    Ponteiro válido, se conseguiu
         NULL, caso contrário:
         -> first==NULL (lista vazia)
         -> it==NULL (iterador invalido)
         */
        thread = (TCB_t*) GetAtIteratorFila2(queue);
        if (thread == NULL) {
            printf("NULL at iterator\n");
            return NULL;
        }
        
        printf("Looking for (%d), checking (%d)\n", tid, thread->tid);
        
        if (thread->tid == tid) {
            printf("Thread(%d) found\n", tid);
            return thread;
        }
        
        /*
         Função:    Seta o iterador da fila para o próximo elemento
         Ret:    ==0, se conseguiu
         !=0, caso contrário (erro, fila vazia ou chegou ao final da fila)
         Fila vazia        => -NXTFILA_VAZIA
         Iterador inválido    => -NXTFILA_ITERINVAL
         Atingido final da fila    => -NXTFILA_ENDQUEUE
         */

        next = NextFila2(queue);
        switch (next) {
            case -NXTFILA_VAZIA:
                printf("Empty queue!\n");
            case -NXTFILA_ITERINVAL:
                printf("Invalid iterator!\n");
            case -NXTFILA_ENDQUEUE:
                printf("Queue end reached!\n");
        }
    } while (next == 0);
    
    printf("Thread %d not found (REALLY!)\n", tid);
    return NULL;
}

int csetprio(int tid, int prio) {
    if (prio < 0 || prio > 2) {
        printf("Invalid priority %d for thread %d", prio, tid);
        return CSETPRIO_ERROR;
    } else {
        running_thread->prio = prio;
        return CSETPRIO_SUCCESS;
    }
}

#define CYIELD_SUCCESS 0
#define CYIELD_ERROR -1
int cyield(void) {
    printf("***************** cyield(%d) *****************\n", running_thread->tid);
    
    TCB_t* thread;
    thread = running_thread;
    thread->state = THREAD_STATE_READY;
    
    if(FirstFila2(&ready[thread->prio]) != 0) {
        printf("Fila de Aptos Vazia, cyield negado!\n");
        return CYIELD_ERROR;
    }
    
    if (AppendFila2(&ready[thread->prio], (void*) thread) != 0) {
        printf("Erro adicionando thread a fila de aptos. cyield negado.\n");
        return CYIELD_ERROR;
    }
    
    running_thread = NULL;
    
    swapcontext(&thread->context, &scheduler_context);
    
    return CYIELD_SUCCESS;
}

// MARK: - CJOIN
#define NOT_TARGETED 0
#define TARGETED 1
int is_thread_targeted(int tid) {
    if(FirstFila2(&joins) != 0) {
        //printf("CJOIN: Fila de Joins vazia ou ERRO\n");
        return NOT_TARGETED;
    }
    do {
        join_t* join = GetAtIteratorFila2(&joins);
        if (join == NULL)
            return NOT_TARGETED;
        if (join->target_thread->tid == tid)
            return TARGETED;
    } while (NextFila2(&joins) == 0);
    return NOT_TARGETED;
}

#define CJOIN_SUCCESS 0
#define CJOIN_THREAD_FINISHED -1
#define CJOIN_THREAD_ALREADY_JOINED -2
#define CJOIN_FAIL -3
int cjoin(int tid) {
    printf("***************** cjoin(%d) *****************\n", tid);
    printf("Called by thread(%d)\n", running_thread->tid);
    // print_all_queues();
    
    if(tid == MAIN_THREAD_ID) {
        printf("Can't join main thread! \n");
        return CJOIN_FAIL;
    }
    
    //checar se thread alvo já tem alguem no aguardo
    if (is_thread_targeted(tid) == TARGETED) {
        printf("Já há uma thread esperando por está\n");
        return CJOIN_THREAD_ALREADY_JOINED;
    }
    
    //achar tcb do tid
    TCB_t* target_thread = NULL;
    int prio = THREAD_PRIORITY_HIGH;
    while ((prio <= THREAD_PRIORITY_LOW) && (target_thread == NULL)) {
        printf("Looking for tid(%d) at priority queue(%d)\n", tid, prio);
        target_thread = find_thread_with_id(tid, &ready[prio]);
        if (target_thread != NULL) {
            printf("Found thread(%d)\n", target_thread->tid);
            break; // just in case
        }
        prio++;
    }
    
    if (target_thread == NULL) {
        printf("Target thread(%d) not found\n", tid);
        return CJOIN_FAIL;
    }
    
    // Create join
    join_t* join = (join_t*) malloc(sizeof(join_t));
    join->blocked_thread = running_thread;
    join->target_thread = target_thread;
    
    AppendFila2(&joins, (void*) join);
    // Se utilizar uma variavel pra controlar o retorno, o teste cjoin da erro
//    int appendError = AppendFila2(&joins, (void*) join);
    //    if (appendError != 0) {
    //        printf("Error(%d): Unable to append join(%d->%d) to queue\n", appendError, running_thread->tid, target_thread->tid);
    //    }

    TCB_t* calling_thread = running_thread;
    calling_thread->state = THREAD_STATE_BLOCKED;
    
    AppendFila2(&blocked, (void*)calling_thread);
    running_thread = NULL;
    swapcontext(&calling_thread->context, &scheduler_context);
    
    return CJOIN_SUCCESS;
    
    // NEW CODE
//
//    appendError = AppendFila2(&blocked, (void*)running_thread);
//    if (appendError == 0) {
//        running_thread->state = THREAD_STATE_BLOCKED;
//    } else {
//        printf("Error(%d): Unable to append join(%d->%d) to queue\n", appendError, join->blocked_thread->tid, join->target_thread->tid);
//    }
//
//    // Vamos liberar a running_thread entao copiamos o ponteiro antes
//    TCB_t* caller_thread = running_thread;
//
//    // Não temos mais ninguem rodando, hora de chamar o schedule
//    running_thread = NULL;
//    swapcontext(&caller_thread->context, &scheduler);
//
//    return CJOIN_SUCCESS;
}

#define CSEM_INIT_SUCCESS 0
#define CSEM_INIT_ERROR_CREATE_QUEUE -1
int csem_init (csem_t *sem, int count) {
    sem->count = count;
    sem->fila = (PFILA2) malloc(sizeof(FILA2));
    
    if (CreateFila2(sem->fila) != 0) {
        printf("Erro ao alocar fila para o semaforo");
        return CSEM_INIT_ERROR_CREATE_QUEUE;
    }
    
    return CSEM_INIT_SUCCESS;
}

#define CWAIT_SUCCESS 0
#define CWAIT_ERROR_CREATE_QUEUE -1
int cwait (csem_t *sem) {
    printf("***************** CWAIT *****************\n");
    initIfNeeded();
    
    if (sem->fila == NULL) {
        sem->fila = (PFILA2) malloc(sizeof(FILA2));
        if (CreateFila2(sem->fila) != 0) {
            //printf("\n");
            return CWAIT_ERROR_CREATE_QUEUE;
        }
    }
    
    sem->count--;
    // sem recursos disponivel
    if (sem->count < 0) {
        TCB_t* thread;
        thread = running_thread;
        thread->state = THREAD_STATE_BLOCKED;
        
        AppendFila2(&blocked, (void*) thread);
        AppendFila2(sem->fila, (void*) thread);
        
        running_thread = NULL;
        
        swapcontext(&thread->context, &scheduler_context);
    }
    
    return CWAIT_SUCCESS;
}

#define MOVE_THREAD_SUCCESS 0
#define MOVE_THREAD_ERROR_OR_EMPTY_QUEUE -1
#define MOVE_THREAD_TID_NOT_FOUND -2
int move_thread(int tid, FILA2 queue, FILA2 dest) {
    if(FirstFila2(&queue) != 0) {
        //printf("Fila de Origem vazia ou ERRO\n");
        return MOVE_THREAD_ERROR_OR_EMPTY_QUEUE;
    }
    TCB_t* thread;
    do {
        thread = GetAtIteratorFila2(&queue);
        if (thread == NULL)
            return MOVE_THREAD_TID_NOT_FOUND;
        
        if (thread->tid == tid) {
            printf("MOVTHREAD: target = %d, found = %d\n",tid, thread->tid );
            printf("Append: %d\n", AppendFila2(&dest, &thread));
            printf("Delete:%d\n", DeleteAtIteratorFila2(&queue));
            return MOVE_THREAD_SUCCESS;
        }
    } while (NextFila2(&queue) == 0);
    
    return MOVE_THREAD_SUCCESS;
}

#define CSIGNAL_SUCCESS 0
#define CSIGNAL_ERROR_UNINITIALIZED -1
#define CSIGNAL_ERROR_REMOVE_THREAD_FROM_BLOCKED -2
int csignal (csem_t *sem) {
    printf("************** CSIGNAL **************\n");
    initIfNeeded();
    
    if (sem->fila == NULL) {
        printf("Signal antes do wait ou semaforo nao inicializado");
        return CSIGNAL_ERROR_UNINITIALIZED;
    }
    
    if (FirstFila2(sem->fila) != 0) {
        FREE(sem->fila);
        return CSIGNAL_SUCCESS;
    }
    
    sem->count++;
    TCB_t* thread;
    thread = (TCB_t*) GetAtIteratorFila2(sem->fila);
    thread->state = THREAD_STATE_READY;
//    print_all_queues();
    printf("SIGNAL VAI LIBERAR THREAD: %d\n", thread->tid);
    DeleteAtIteratorFila2(sem->fila);
    
    if (move_thread(thread->tid, blocked, ready[thread->prio]) != 0) {
        printf("Erro ao remover thread(%d) da fila de bloqueados", thread->tid);
        return CSIGNAL_ERROR_REMOVE_THREAD_FROM_BLOCKED;
    }
//    print_all_queues();
    return CSIGNAL_SUCCESS;
}

#define CIDENTIFY_SUCCESS 0
#define CIDENTIFY_ERROR -1
int cidentify(char *name, int size) {
    char * result = strncpy(name, "Henrique Valcanaia 240501\nLucas Bauer - 237054\nAlvaro Souza Pereira da Silva - 231162\n", size);
    return result == NULL ? CIDENTIFY_ERROR : CIDENTIFY_SUCCESS;
    
}


