//
//  debug.h
//  cthread
//
//  Created by Henrique Valcanaia on 13/04/19.
//  Copyright © 2019 Henrique Valcanaia. All rights reserved.
//

#ifndef debug_h
#define debug_h

// MARK: - Debug
#define DEBUG 3

#if defined(DEBUG) && DEBUG > 0
//#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %s(): " fmt, __func__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif


void print_all_queues();
void print_joins();
void print_queue(FILA2 queue);

#endif /* debug_h */
