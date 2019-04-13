//
//  test_cjoin.c
//  cthread
//
//  Created by Henrique Valcanaia on 15/09/16.
//  Copyright © 2016 Henrique Valcanaia. All rights reserved.
//

#include <stdio.h>
#include "../include/cthread.h"
#define _XOPEN_SOURCE 600
#define THREAD_COUNT 20 

void* func3(void *arg);
void* func1(void *arg);
void* func2(void *arg);

void* func3(void *arg) {
    printf("inicio thread TRES\n");
    printf("fim da thread TRES\n");
    return 0;
}

void* func1(void *arg) {
    printf("inicio thread UM\n");
    int id2 = ccreate(func2, NULL, THREAD_PRIORITY_HIGH);
    cjoin(id2);
    printf("fim da thread UM\n");
    return 0;
}

void* func2(void *arg) {
    printf("inicio thread DOIS\n");
    int id3 = ccreate(func3, NULL, THREAD_PRIORITY_HIGH);
    cjoin(id3);
    printf("fim da thread DOIS\n");
    return 0;
}

int main(int argc, char *argv[]) {
    int id1 = ccreate(func1, NULL, THREAD_PRIORITY_HIGH);
    cjoin(id1);
    printf("Main chegou ao fim.\n");    
    return 0;
}
