//
//  test_cwait.c
//  cthread
//
//  Created by Henrique Valcanaia on 29/09/16.
//  Copyright © 2016 Henrique Valcanaia. All rights reserved.
//

#include "../include/cthread.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    csem_t* sem;
    csem_init(sem, 5);
    
    csignal(sem);
    
    cwait(sem);
}
