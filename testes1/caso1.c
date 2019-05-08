
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

#define BAIXA	2
#define MEDIA	1
#define ALTA	0


/*-------------------------------------------------------------------
Operação para Teste
	operacao==0	-> operacao normal
	operacao==1	-> retorna SEMPRE o valor de tempo
	operacao==2	-> reseta operacao e retorna o valor de tempo (one shot)
-------------------------------------------------------------------*/
void	setStopTimer(int op, int tm);

void *th(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
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
	printf ("OPERACAO:\n");
	printf ("     main (prio=baixa), so faz yield()\n");
	printf ("     create 3 threads (prio=baixa), que nao fazem yield()\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Roda apenas uma das threads de cada vez\n");
	printf ("     11111111...2222222...33333333...\n");	
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();
	
	//setStopTimer(1, 1000);	// todos terão a mesma prioridade
	
	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);

	tid[0] = ccreate(th, (void *)1, BAIXA);
	if (tid[0]<0) {printf ("Erro no ccreate(1,0).\n"); goto finish;}
	
	tid[1] = ccreate(th, (void *)2, BAIXA);
	if (tid[1]<0) {printf ("Erro no ccreate(2,0).\n"); goto finish;}
	
	tid[2] = ccreate(th, (void *)3, BAIXA);
	if (tid[2]<0) {printf ("Erro no ccreate(3,0).\n"); goto finish;}

	while(--delay) {
		cyield();
	}

finish:	
	printf ("Fim do main\n");
	return 0;
}





