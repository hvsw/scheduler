/*
 * cthread.h: arquivo de inclus„o com os protÛtipos das funÁıes a serem
 *            implementadas na realizaÁ„o do trabalho.
 *
 * N√O MODIFIQUE ESTE ARQUIVO.
 *
 * VERS√O: 29/03/2019
 *
 */
#ifndef __cthread__
#define __cthread__

#define CSETPRIO_SUCCESS 0
#define CSETPRIO_ERROR -1

#include "support.h"

typedef struct s_sem {
	int	count;	/* indica se recurso est· ocupado ou n„o (livre > 0, ocupado = 0) */
	PFILA2	fila; 	/* ponteiro para uma fila de threads bloqueadas no sem·foro */
} csem_t;

/*!
 @brief Efetua cedencia voluntária de CPU
 @param start ponteiro para a função que a thread executará.
 @param arg um parâmetro que pode ser passado para a thread na sua criação. (Obs.: é um único parâmetro. Se for necessário passar mais de um valor deve-se empregar um ponteiro para uma struct)
 @return Quando executada corretamente retorna um valor positivo, que representa o identificador da thread criada, caso contrário, retorna CCREATE_ERROR (valor negativo)
 */
int ccreate (void* (*start)(void*), void *arg, int prio);

/*!
 @brief Efetua cedencia voluntária de CPU
 @return Quando executada corretamente retorna CYIELD_SUCCESS (0), caso contrário, retorna CYIELD_ERROR (-1)
 */
int cyield(void);

/*!
 @brief Altera a prioridade com id = tid
 @param tid identificador da thread cuja prioridade será alterada (deixar sempre esse campo como NULL em 2018/02)
 @param prio nova prioridade da thread
 @return Quando executada corretamente retorna CSETPRIO_SUCCESS (0), caso contrário, retorna CSETPRIO_ERROR (-1)
*/
int csetprio(int tid, int prio);


/*!
 @brief Bloqueia a execução de uma thread aguardando o término da thread com id indicado no parametro 'tid'
 @discussion Exemplo para geração de um tid válido
 
 @code
 int id0, id1, i;
 id0 = ccreate(func0, (void *)&i);
 id1 = ccreate(func1, (void *)&i);
 
 printf("Eu sou a main após a criação de ID0 e ID1\n");
 
 cjoin(id0);
 cjoin(id1);
 
 printf("Eu sou a main voltando para terminar o programa\n");
 @endcode
 
 
 @param tid identificador da thread cujo término está sendo aguardado. Caso seja informado um tid inválido a função retornará um valor negativo indicando erro.
 @return Quando executada corretamente retorna CJOIN_SUCCESS (0 zero). Caso contrário, retorna CJOIN_ERROR (um valor negativo)
 */
int cjoin(int tid);

/*!
 @param sem ponteiro para uma variável do tipo csem_t. Aponta para uma estrutura de dados que representa a variável semáforo.
 @param count valor a ser usado na inicialização do semáforo. Representa a quantidade de recursos controlador pelo semáforo.
 @return Quando executada corretamente: retorna 0 (zero) Caso contrário, retorna um valor negativo.
 */
int csem_init(csem_t *sem, int count);

/*!
 @param sem ponteiro para uma variável do tipo semáforo.
 @return Quando executada corretamente retorna CWAIT_SUCCESS (0 zero), caso contrário, retorna CWAIT_ERROR (um valor negativo)
 */
int cwait(csem_t *sem);

/*!
 @param sem ponteiro para uma variável do tipo semáforo.
 @return Quando executada corretamente retorna CSIGNAL_SUCCESS (0 zero), caso contrário, retorna CSIGNAL_ERROR (um valor negativo)
 */
int csignal(csem_t *sem);

/*!
 @param name ponteiro para uma área de memória onde deve ser escrito um string que contém os nomes dos componentes do grupo e seus números de cartão. Deve ser uma linha por componente.
 @param size quantidade máxima de caracteres que podem ser copiados para o string de identificação dos componentes do grupo.
 @return Quando executada corretamente retorna CIDENTIFY_SUCCESS (0 zero), caso contrário, retorna CIDENTIFY_ERROR(um valor negativo)
 */
int cidentify (char *name, int size);


#endif

