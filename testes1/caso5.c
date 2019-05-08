
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


void *th2(void *param) {
	int n=(int)param;
	int cont=50;
	while(cont) {
		printf ("%d",n);
		--cont;
//		setStopTimer(1, PRIO_LOW);
		cyield();
	}
	printf ("Fim da th2\n");
	return NULL;
}

int main(int argc, char *argv[]) {
	char	name[256];
	int	id2;
	
	printf ("FUNCOES TESTADAS:\n");
	printf ("     identify()\n");
	printf ("     create()\n");
	printf ("     join()\n");
	printf ("OPERACAO:\n");
	printf ("     main (HIGH), cria th2 (LOW) e faz join na th2.\n");
	printf ("     th2 fica fazendo yield()\n");
//	printf ("     USA setStopTimer para controlar, indiretamente, a prioridade\n");
	printf ("RESULTADO ESPERADO\n");
	printf ("     Roda apenas thread 2, pois a main esta esperando o final da th2\n");
	printf ("     th2 termina, desbloqueia a thread main e termina o programa\n");
	printf ("     2222222...Fim da th2\n");
	printf ("     SE HOUVER ERRO, o programa vai travar\n");
	printf ("DIGITE ALGO PARA INICIAR\n");	
	getchar();
	
	cidentify (name, 255);
	printf ("GRUPO: %s\n\n", name);	

        csetprio((int)NULL,ALTA);
	
//	setStopTimer(1, PRIO_LOW);
	if ( (id2=ccreate(th2, (void *)2, 1))<0) {
		printf ("Erro no ccreate(th1, , 1)\n");
	} else {
//		setStopTimer(1, PRIO_HIGH);	// Main tem a maior prioridade, mas so vai rodar quando a th2 terminar
		if (cjoin(id2)<0) {
			printf ("Erro no cjoin(th2).\n");
		}
	}
	printf ("Fim do main\n");
	return 0;
}


