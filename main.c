#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 20

int cont_pizza = 0;
int cont_prontas = 0;
int fim = 0;
int preparando = 0;
int montando_pizza = 0;
int pacotes = 100;
int pizza_pronta = 0;

char comando[20];

pthread_t recebe, massa, montagem, forno, empacota, entrega, show;

pthread_mutex_t contador = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t comando_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t preparacao = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t montagem_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t forno_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t empacota_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tela = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t novo_pedido = PTHREAD_COND_INITIALIZER;
pthread_cond_t massa_pronta = PTHREAD_COND_INITIALIZER;
pthread_cond_t montagem_pronta = PTHREAD_COND_INITIALIZER;
pthread_cond_t forno_pronto = PTHREAD_COND_INITIALIZER;
pthread_cond_t pedido_pronto = PTHREAD_COND_INITIALIZER;

void recebe_pedido() {
    // Prepara o standard input para receber caracteres sem echo e sem enter
	static struct termios oldt, newt;
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);    
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    // Comeca a receber caracteres
	comando[0] = '\0';
	
	int pos = 0;
	int i;
	char c;
	while(1)
	{
		// Compoe o comando caracter a caracter
		while((c = getchar()) != '\n')
		{
			pthread_mutex_lock(&comando_mutex);
			comando[pos++] = c;
			comando[pos] = '\0';
			putchar(c);
			pthread_mutex_unlock(&comando_mutex);
		}
		
		
		pthread_mutex_lock(&comando_mutex);
		
		// Limpa parte da tela (durante exclusao mutua)
		for(i=0; pos > i; i++) putchar('\b');
		for(;pos > 0; pos--) printf(" ");
		
		char substring[BUF_SIZE];
		strncpy(substring, comando, 4);
		substring[4] = '\0';

		// Interpreta o comando
		if(strcmp(substring, "exit") == 0)
		{
			fim = 1;
			pthread_mutex_unlock(&comando_mutex);
			break;
			
		} else if(strcmp(substring, "novo") == 0)
		{
            printf("teclou novo");
			strncpy(substring, comando+4, BUF_SIZE);
			float novoTref = atof(substring);
			pthread_mutex_lock(&contador);
                cont_pizza++;
                pthread_cond_signal(&novo_pedido);
            pthread_mutex_unlock(&contador);
            printf("\n entrou mutex e o valor do cont eh %d",cont_pizza);
		}
		
		comando[0] = '\0';
		
		
		pthread_mutex_unlock(&comando_mutex);
	}
	
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
	pthread_exit(NULL);
}

void prepara_massa() {
    while(cont_pizza < 1) {
        pthread_cond_wait(&novo_pedido,&contador);
    }
    printf("\n entrou reparando massa");
    pthread_mutex_lock(&preparacao);
        printf("\nPreparando massa");
        preparando = 1;
        sleep(1);
        pthread_cond_signal(&massa_pronta);
    pthread_mutex_unlock(&preparacao);
}

void montagem_pizza() {
    while(preparando < 1) {
        pthread_cond_wait(&massa_pronta,&preparacao);
    }
    printf("\n entrou montagem massa");

    pthread_mutex_lock(&montagem_mutex);
        printf("\nMontando pizza");
        montando_pizza = 1;
        pthread_cond_signal(&montagem_pronta);
    pthread_mutex_unlock(&montagem_mutex);
        printf("\n sai montando massa");

}

void coloca_forno() {
     while(montando_pizza < 1) {
        pthread_cond_wait(&montagem_pronta,&montagem_mutex);
    }
    printf("\n entrou forno");

    pthread_mutex_lock(&forno_mutex);
        printf("\n Forno");
        pizza_pronta = 1;
        pthread_cond_signal(&forno_pronto);
        printf("\n sinalzin");
    pthread_mutex_unlock(&forno_mutex);
        printf("\n sai forno");

}

void empacotando() {
    while(pizza_pronta < 1) {
        pthread_cond_wait(&forno_pronto,&forno_mutex);
    }
    printf("\n empacotando");

    pthread_mutex_lock(&contador);
        printf("\n protegi");
    cont_pizza--;
    cont_prontas++;
    pthread_mutex_unlock(&contador);

    pthread_mutex_lock(&empacota_mutex);
        printf("\n Empacota");
        pizza_pronta = 0;
        pthread_cond_signal(&pedido_pronto);
    pthread_mutex_unlock(&empacota_mutex);
        printf("\n sai empacota");
}

void mostra_tela () {
    while(1) {
        pthread_mutex_lock(&tela);
        system("clear");
	    printf("*-*-*-*-  Pizzaria da Ana e da Marina    *-*-*-*-\n\n");
        if (preparando > 0) {
            pthread_mutex_lock(&preparacao);
            printf("A pizza esta sendo preparada");
            pthread_mutex_unlock(&preparacao);
        }
        printf("No momento temos %d pedidos \n",cont_pizza);
        printf("Ja preparamos %d pedidos \n",cont_pizza);
        printf("Ja temos %d pacotes \n",pacotes);
        pthread_mutex_unlock(&tela);
        printf("Para preparar uma pizza, digite 'novo': \n");
        sleep(5);
    }

}

void entrega_final() {}

void main() {
    printf("*Pizzaria Ana e Marina* \n");
    printf("Tecle novo para adicionar um pedido \n");
    pthread_create(&recebe, NULL, (void*)recebe_pedido, NULL);
    pthread_create(&massa, NULL, (void*)prepara_massa, NULL);
    pthread_create(&montagem, NULL, (void*)montagem_pizza, NULL);
    pthread_create(&forno, NULL, (void*)coloca_forno, NULL);
    pthread_create(&empacota, NULL, (void*)empacotando, NULL);
    pthread_create(&show, NULL, (void*)mostra_tela, NULL);

    pthread_join(recebe,NULL);
    pthread_join(massa,NULL);
    pthread_join(montagem,NULL);
    pthread_join(forno,NULL);
    pthread_join(empacota,NULL);
    pthread_join(show,NULL);


}
