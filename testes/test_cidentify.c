//
//  test_cidentify.c
//  cthread
//
//  Created by Henrique Valcanaia on 15/09/16.
//  Copyright © 2016 Henrique Valcanaia. All rights reserved.
//

#include "test_cidentify.h"
#include "../include/cthread.h"

#define NAMES_BUFFER 100

int main(int argc, char *argv[]) {
    test();
}

void test() {
    char *nomes = (char*) malloc(NAMES_BUFFER *(sizeof(char)));
    int result = cidentify(nomes, NAMES_BUFFER);
    if (result == -1) {
        printf("Erro ao copiar as coisinhas");
    } else {
        printf("%s", nomes);
    }
}
