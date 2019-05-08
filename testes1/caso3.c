
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

#define BAIXA 2
#define MEDIA 1
#define ALTA  0


/*-------------------------------------------------------------------
Operação para Teste
	operacao==0	-> operacao normal
	operacao==1	-> retorna SEMPRE o valor de tempo
	operacao==2	-> reseta operacao e retorna o valor de tempo (one shot)
-------------------------------------------------------------------*/
void	setStopTimer(int op, int tm);

#define	PRIO_LOW	1000
#define	PRIO_HIGH	500


void *th1(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
//		setStopTimer(1, PRIO_LOW);	// Menor prioridade
		cyield();
	}
	return NULL;
}

void *th2(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
//		setStopTimer(1, PRIO_HIGH);	// Maior prioridade
		// cyield();		// nao faz yield, para trancar nessa thread
	}
	return NULL;
}

void *th3(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
//		setStopTimer(1, PRIO_LOW);	// Menor prioridade
		cyield();
	}
	return NULL;
}



int main(int argc, char *argv[]) {
	int	delay=10000;
	char	name[256];
	int	tid[3];
	
	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     create()\n");
	printf ("     parcialmente, yield()\n");
	printf ("     mecanismo de prioridades\n");
	printf ("OPERACAO:\n");
	printf ("     main (prio=HIGH), so faz yield()\n");
	printf ("     create thread=2 (prio=HIGH), que NAO FAZ yield()\n");
	printf ("     create threads=1 e 3 (prio=LOW), que FAZEM yield()\n");
//	printf ("     Usado setStopTimer, para atribuir, indiretamente, as prioridades\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Roda apenas a thread=3\n");
	printf ("     As outras nao rodam porque a main encerra\n");
	printf ("     333333....\n");
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();

	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);	

        csetprio((int)NULL, ALTA);
	
//	setStopTimer(1, PRIO_HIGH);
	tid[0] = ccreate(th1, (void *)1, BAIXA);
	if (tid[0]<0) {printf ("Erro no ccreate(1,1).\n"); goto finish;}
	
//	setStopTimer(1, PRIO_HIGH);
	tid[1] = ccreate(th2, (void *)2, MEDIA);
	if (tid[1]<0) {printf ("Erro no ccreate(2,1).\n"); goto finish;}
	
//	setStopTimer(1, PRIO_HIGH);
	tid[2] = ccreate(th3, (void *)3, ALTA);
	if (tid[2]<0) {printf ("Erro no ccreate(3,1).\n"); goto finish;}
	
	while(--delay) {
//		setStopTimer(1, PRIO_HIGH);	// Main tem a mesma prioridade da th2 (e maior que as th1 e th3)
		cyield();
	}
	
finish:	
	printf ("Fim do main\n");
	return 0;
}





