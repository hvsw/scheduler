//
//  main.c
//  cthread
//
//  Created by Henrique Valcanaia on 02/04/19.
//  Copyright © 2019 Henrique Valcanaia. All rights reserved.
//

#include <stdio.h>
#include "include/cthread.h"
#define _XOPEN_SOURCE 600

int main(int argc, const char * argv[]) {
    
    int nameSize = 20;
    char name[nameSize];
    if (cidentify(name, nameSize) > 0) {
        printf("%s", name);
    }
    
//    DEBUG_PRINT("go satan");
    
    return 0;
}
