
#define _XOPEN_SOURCE 600
#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"
#include <string.h>
#include <stdlib.h>
#include "debug.h"

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
ucontext_t scheduler;
TCB_t main_thread;

// MARK: Private helpers
void destroy_join(join_t* join) {
    //DEBUG_PRINT("vai destruir join entre %d e %d\n",join->blocked_thread->tid, join->target_thread->tid);
    if(FirstFila2(&blocked) != 0) {
        //DEBUG_PRINT("Fila de Aptos vazia ou ERRO\n");
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
    return;
}

void release_threads_from_tid(int tid) {
    if(FirstFila2(&joins) != 0) {
        //DEBUG_PRINT("SCHD: Fila de Joins vazia ou ERRO\n");
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
    return;
}

TCB_t* find_next_thread() {
    TCB_t* thread;
    int priority;
    for (priority = THREAD_PRIORITY_HIGH; priority > THREAD_PRIORITY_LOW; priority--) {
        FILA2 queue = ready[priority];
        int set_iterator = FirstFila2(&queue);
        if (set_iterator == 0) {
            TCB_t* thread = (TCB_t*)GetAtIteratorFila2(&queue);
            if (thread != NULL) {
                return thread;
            }
        } else {
            printf("Error while trying to set iterator to first position");
        }
    }
    
    return NULL;
}

#define REMOVE_THREAD_SUCCESS 0
#define REMOVE_THREAD_ERROR_OR_EMPTY_QUEUE -1
#define REMOVE_THREAD_TID_NOT_FOUND -2
int remove_thread(int tid, FILA2 queue) {
    if(FirstFila2(&queue) != 0) {
        //DEBUG_PRINT("Fila vazia ou ERRO\n");
        return REMOVE_THREAD_ERROR_OR_EMPTY_QUEUE;
    }
    
    TCB_t* thread;
    do {
        thread = GetAtIteratorFila2(&queue);
        if (thread == NULL) {
            //DEBUG_PRINT("tid %d not found", tid);
            return REMOVE_THREAD_TID_NOT_FOUND;
        }
        
        if (thread->tid == tid) {
            //DEBUG_PRINT("REMOVTHREAD: target = %d, found = %d\n",tid, thread->tid );
            int success = DeleteAtIteratorFila2(&queue);
            //DEBUG_PRINT("Delete: %d\n", success);
            return REMOVE_THREAD_SUCCESS;
        }
    } while (NextFila2(&queue) == 0);
    
    return REMOVE_THREAD_SUCCESS;
}

void schedule() {
    //DEBUG_PRINT("***************** %s *****************\n", __func__);
//    print_all_queues();
    
    //se running thread nula, quer dizer que foi yield. logo, se não nula devemos assumir que a thread encerrou
    if (running_thread != NULL) {
        //DEBUG_PRINT("Running Thread Existe; Assume que a thread encerrou\n");
        release_threads_from_tid(running_thread->tid);
        running_thread = NULL;
    }
    
    int readyQueuesEmptyOrError = (FirstFila2(&ready[THREAD_PRIORITY_LOW]) != 0);
    readyQueuesEmptyOrError = readyQueuesEmptyOrError || (FirstFila2(&ready[THREAD_PRIORITY_MEDIUM]) != 0);
    readyQueuesEmptyOrError = readyQueuesEmptyOrError || (FirstFila2(&ready[THREAD_PRIORITY_HIGH]) != 0);
    if (readyQueuesEmptyOrError) {
        return;
    }
    
    
    TCB_t* next_thread = find_next_thread();
    //DEBUG_PRINT("next_thread: tid(%d)\n", next_thread->tid);
    // print_all_queues();
    
    running_thread = next_thread;
    if (remove_thread(next_thread->tid, ready[next_thread->prio]) != 0) {
        //DEBUG_PRINT("problema ao deletar thread corrente da fila de aptos\n");
    }
    
    running_thread->state = THREAD_STATE_RUNNING;
    // print_all_queues();
    
    //DEBUG_PRINT("Running thread: %d", running_thread->tid);
    
    setcontext(&running_thread->context);
}

int generate_thread_id() {
    currentThreadId += 1;
    return currentThreadId;
}

// MARK: Init
char ss_sp_scheduler[SIGSTKSZ];
void init_scheduler() {
    getcontext(&scheduler);
    scheduler.uc_link = &(main_thread.context);
    scheduler.uc_stack.ss_sp = ss_sp_scheduler;
    scheduler.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&scheduler, (void (*)(void))schedule, 0);
}

void create_main_tcb() {
    main_thread.tid = MAIN_THREAD_ID;
    main_thread.state = THREAD_STATE_RUNNING;
    getcontext(&main_thread.context);
    
    running_thread = &main_thread;
}

void init_queues() {
    int priority = 0;
    while (priority++ < NUMBER_PRIORITY_QUEUES) {
        CreateFila2(&ready[priority]);
    }
    CreateFila2(&blocked);
    CreateFila2(&joins);
}

void init() {
    init_scheduler();
    create_main_tcb();
    init_queues();
    initialized = 1;
}

// MARK: Public methods
int ccreate (void* (*start)(void*), void *arg, int prio) {
    return NOT_IMPLEMENTED;
    
    if (initialized == 0) {
        init();
    }
    
    ucontext_t context;
    if (getcontext(&context) == 0) {
        char tcb_stack[SIGSTKSZ];
        context.uc_link = &scheduler;
        context.uc_stack.ss_sp = tcb_stack;
        context.uc_stack.ss_size = SIGSTKSZ;
        makecontext(&context, (void (*)(void)) start, 1, arg);
        
        TCB_t* tcb = (TCB_t*) malloc(sizeof(TCB_t));
        tcb->state = THREAD_STATE_CREATION;
        tcb->tid = generate_thread_id();
        tcb->context = context;
        tcb->prio = prio;
        if (AppendFila2(&ready[prio], (void*) tcb) == 0) {
//            DEBUG_PRINT("Adicionou na fila %d\n", tcb->tid);
            return tcb->tid;
        } else {
//            DEBUG_PRINT("Erro ao adicionar na fila");
            return CCREATE_ERROR;
        }
    } else {
//        DEBUG_PRINT("Erro ao obter context");
        return CCREATE_ERROR;
    }
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

#define CIDENTIFY_SUCCESS 0
#define CIDENTIFY_ERROR -1
int cidentify(char *name, int size) {
    char * result = strncpy(name, "Henrique Valcanaia 240501\nLucas Bauer - 237054\nAlvaro Souza Pereira da Silva - 231162\n", size);
    return result == NULL ? CIDENTIFY_ERROR : CIDENTIFY_SUCCESS;
    
}


