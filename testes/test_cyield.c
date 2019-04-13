//
//  test_cyield.c
//  cthread
//
//  Created by Henrique Valcanaia on 15/09/16.
//  Copyright © 2016 Henrique Valcanaia. All rights reserved.
//

#include <stdio.h>
#include "../include/cthread.h"

void* func3(void *arg);
void* func1(void *arg);
void* func2(void *arg);

void* func3(void *arg) {
    printf("inicio thread 3\n");
    printf("thread 3 vai dar yield.\n");
    cyield();
    printf("fim da thread 3\n");
    return 0;
}

void* func1(void *arg) {
    printf("inicio thread 1.\n");
    ccreate(func2, NULL, THREAD_PRIORITY_HIGH);
    printf("Thread 1: criou thread 2 e vai dar yield.\n");
    cyield();
    printf("fim da thread UM\n");
    return 0;
}

void* func2(void *arg) {
    printf("inicio thread 2\n");
    ccreate(func3, NULL, THREAD_PRIORITY_HIGH);
    printf("Thread 2: criou thread 3 e vai dar yield.\n");
    cyield();
    printf("fim da thread DOIS\n");
    return 0;
}

int main(int argc, char *argv[]) {
    ccreate(func1, NULL, THREAD_PRIORITY_HIGH);
    printf("Main criou Thread 1 e vai dar yield.\n");
    cyield();

    printf("Main recebeu o processador e vai dar yield.\n");
    cyield();
    

    printf("Main recebeu o processador e vai dar yield.\n");
    cyield();
    
    printf("Main recebeu o processador e vai dar yield.\n");
    cyield();

    printf("Main chegou ao fim.\n");    
    return 0;
}
