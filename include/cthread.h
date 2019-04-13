/*
 * cthread.h: arquivo de inclus�o com os prot�tipos das fun��es a serem
 *            implementadas na realiza��o do trabalho.
 *
 * N�O MODIFIQUE ESTE ARQUIVO.
 *
 * VERS�O: 29/03/2019
 *
 */
#ifndef __cthread__
#define __cthread__

#define CSETPRIO_SUCCESS 0
#define CSETPRIO_ERROR -1

#include "support.h"

typedef struct s_sem {
	int	count;	/* indica se recurso est� ocupado ou n�o (livre > 0, ocupado = 0) */
	PFILA2	fila; 	/* ponteiro para uma fila de threads bloqueadas no sem�foro */
} csem_t;

/*!
 @brief Efetua cedencia volunt�ria de CPU
 @param start ponteiro para a fun��o que a thread executar�.
 @param arg um par�metro que pode ser passado para a thread na sua cria��o. (Obs.: � um �nico par�metro. Se for necess�rio passar mais de um valor deve-se empregar um ponteiro para uma struct)
 @return Quando executada corretamente retorna um valor positivo, que representa o identificador da thread criada, caso contr�rio, retorna CCREATE_ERROR (valor negativo)
 */
int ccreate (void* (*start)(void*), void *arg, int prio);

/*!
 @brief Efetua cedencia volunt�ria de CPU
 @return Quando executada corretamente retorna CYIELD_SUCCESS (0), caso contr�rio, retorna CYIELD_ERROR (-1)
 */
int cyield(void);

/*!
 @brief Altera a prioridade com id = tid
 @param tid identificador da thread cuja prioridade ser� alterada (deixar sempre esse campo como NULL em 2018/02)
 @param prio nova prioridade da thread
 @return Quando executada corretamente retorna CSETPRIO_SUCCESS (0), caso contr�rio, retorna CSETPRIO_ERROR (-1)
*/
int csetprio(int tid, int prio);


/*!
 @brief Bloqueia a execu��o de uma thread aguardando o t�rmino da thread com id indicado no parametro 'tid'
 @discussion Exemplo para gera��o de um tid v�lido
 
 @code
 int id0, id1, i;
 id0 = ccreate(func0, (void *)&i);
 id1 = ccreate(func1, (void *)&i);
 
 printf("Eu sou a main ap�s a cria��o de ID0 e ID1\n");
 
 cjoin(id0);
 cjoin(id1);
 
 printf("Eu sou a main voltando para terminar o programa\n");
 @endcode
 
 
 @param tid identificador da thread cujo t�rmino est� sendo aguardado. Caso seja informado um tid inv�lido a fun��o retornar� um valor negativo indicando erro.
 @return Quando executada corretamente retorna CJOIN_SUCCESS (0 zero). Caso contr�rio, retorna CJOIN_ERROR (um valor negativo)
 */
int cjoin(int tid);

/*!
 @param sem ponteiro para uma vari�vel do tipo csem_t. Aponta para uma estrutura de dados que representa a vari�vel sem�foro.
 @param count valor a ser usado na inicializa��o do sem�foro. Representa a quantidade de recursos controlador pelo sem�foro.
 @return Quando executada corretamente: retorna 0 (zero) Caso contr�rio, retorna um valor negativo.
 */
int csem_init(csem_t *sem, int count);

/*!
 @param sem ponteiro para uma vari�vel do tipo sem�foro.
 @return Quando executada corretamente retorna CWAIT_SUCCESS (0 zero), caso contr�rio, retorna CWAIT_ERROR (um valor negativo)
 */
int cwait(csem_t *sem);

/*!
 @param sem ponteiro para uma vari�vel do tipo sem�foro.
 @return Quando executada corretamente retorna CSIGNAL_SUCCESS (0 zero), caso contr�rio, retorna CSIGNAL_ERROR (um valor negativo)
 */
int csignal(csem_t *sem);

/*!
 @param name ponteiro para uma �rea de mem�ria onde deve ser escrito um string que cont�m os nomes dos componentes do grupo e seus n�meros de cart�o. Deve ser uma linha por componente.
 @param size quantidade m�xima de caracteres que podem ser copiados para o string de identifica��o dos componentes do grupo.
 @return Quando executada corretamente retorna CIDENTIFY_SUCCESS (0 zero), caso contr�rio, retorna CIDENTIFY_ERROR(um valor negativo)
 */
int cidentify (char *name, int size);


#endif

