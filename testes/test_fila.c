//
//  test_fila.c
//  cthread
//
//  Created by Henrique Valcanaia on 2019-05-6.
//  Copyright © 2019 Henrique Valcanaia. All rights reserved.
//

#include "test_fila.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILA2 filaDeIntFDP;
    int criouEstaMerda = CreateFila2(&filaDeIntFDP);
    printf("%d\n", criouEstaMerda);
    
    int content = 1;
    
    int appendou = AppendFila2(&filaDeIntFDP, (void*)content);
    printf("%d\n", appendou);
    
    content = 2;
    appendou = AppendFila2(&filaDeIntFDP, (void*)content);
    printf("%d\n", appendou);
    
    content = 3;
    appendou = AppendFila2(&filaDeIntFDP, (void*)content);
    printf("%d\n", appendou);
}
