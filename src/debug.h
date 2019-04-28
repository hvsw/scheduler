//
//  debug.h
//  cthread
//
//  Created by Henrique Valcanaia on 13/04/19.
//  Copyright © 2019 Henrique Valcanaia. All rights reserved.
//

#ifndef debug_h
#define debug_h

#include <stdio.h>
#include <stdlib.h>
#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/support.h"

// MARK: - Debug
// SE ESTIVER OCORRENDO FALHA DE SEGMENTAÇÃO/SEGMENTATION FAULT,
// COMENTE A LINHA QUE DEFINE A VARIAVEL "DEBUG"!
// Isso ocorre pois ao tentar voltar pra thread main, não temos
// mais a referencia de __func__, causando uma falha de segmentação

#define DEBUG 3

#if defined(DEBUG) && DEBUG > 0
//#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s(): " fmt, __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif


void print_all_queues(void);
void print_joins(void);
void print_queue(FILA2 queue);

#endif /* debug_h */
