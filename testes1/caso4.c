
#include <stdio.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"

/*-------------------------------------------------------------------
Operação para Teste
	operacao==0	-> operacao normal
	operacao==1	-> retorna SEMPRE o valor de tempo
	operacao==2	-> reseta operacao e retorna o valor de tempo (one shot)
-------------------------------------------------------------------*/
void	setStopTimer(int op, int tm);

#define BAIXA 2
#define MEDIA 1
#define ALTA  0

int	tid[3];

void *th3(void *param) {
        int n=(int)param;
   
        printf("%d",n);
        return NULL;
}

void *th2(void *param) {
        int n=(int)param;
   
        printf("%d",n);
	tid[2] = ccreate(th3, (void *)3, ALTA);
	if (tid[2]<0) {printf ("Erro no ccreate(2,1).\n"); printf ("Fim do main\n");exit(0);}
        return NULL;
}

void *th(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
	        tid[1] = ccreate(th2, (void *)2, MEDIA);
	        if (tid[1]<0) {printf ("Erro no ccreate(2,1).\n");printf ("Fim do main\n");exit(0);}
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	int	delay=10000;
	char	name[256];
	
	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     create()\n");
	printf ("     yield()\n");
	printf ("OPERACAO:\n");
	printf ("     main cria 3 threads de prio igual a da main e so faz yield()\n");
	printf ("     threads sempre fazem yield()\n");
//	printf ("     Usado setStopTimer para setar todas as prioridades iguais\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Uma thread de prioridade mais baixa cria uma de mais alta prioridade.\n");
        printf ("     cada uma de prioriade media ou alta, imprime o seu id, faz yield e termina.\n");
	printf ("     123123123123...\n");
	printf ("DIGITE ALGO PARA INICIAR\n");
	getchar();

//	setStopTimer(1, 1000);	// todos terão a mesma prioridade

	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);	
	
	tid[0] = ccreate(th, (void *)1, BAIXA);
	if (tid[0]<0) {printf ("Erro no ccreate(1,1).\n"); printf ("Fim do main\n");exit(0);}

        cyield();
	
	while(--delay) {
		cyield();
	}

	printf ("Fim do main\n");
	return 0;
}





