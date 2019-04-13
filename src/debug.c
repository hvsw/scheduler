//
//  debug.c
//  cthread
//
//  Created by Henrique Valcanaia on 13/04/19.
//  Copyright © 2019 Henrique Valcanaia. All rights reserved.
//

#include "debug.h"

// MARK: - Helpers
void print_queue(FILA2 queue) {
    if (FirstFila2(&queue) != 0) {
        //DEBUG_PRINT("Fila vazia\n");
        return;
    }
    
    TCB_t* currentTCB;
    do {
        currentTCB = GetAtIteratorFila2(&queue);
        if (currentTCB == NULL) {
            break;
        }
        
        //DEBUG_PRINT("tid(%d) -- ticket(%d)\n", currentTCB->tid, currentTCB->ticket);
    } while (NextFila2(&queue) == 0);
}

void print_joins() {
    //    if(FirstFila2(&joins) != 0) {
    //        //DEBUG_PRINT("-\n");
    //        return;
    //    }
    //
    //    join_t* join;
    //    do {
    //        join = GetAtIteratorFila2(&joins);
    //        if (join == NULL)
    //            return;
    //        //DEBUG_PRINT("(%d) (%d)\n", join->blocked_thread->tid, join->target_thread->tid);
    //    } while (NextFila2(&joins) == 0);
    //    return;
}

void print_all_queues() {
    //DEBUG_PRINT("Ready\n");
    print_queue(ready);
    //DEBUG_PRINT("Blocked\n");
    print_queue(blocked);
    //DEBUG_PRINT("Joins\n");
    print_joins();
}
