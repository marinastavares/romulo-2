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

int cont_pizza=0;
int fim =0;
int preparando = 0;
int montando_pizza = 0;
int pacotes = 100;
int pizza_pronta = 0;

char comando[20];

pthread_t recebe, massa, montagem, forno, pacote, entrega;

pthread_mutex_t contador = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t comando_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t preparacao = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tela = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t novo_pedido = PTHREAD_COND_INITIALIZER;
pthread_cond_t massa_pronta = PTHREAD_COND_INITIALIZER;
pthread_cond_t montagem_pronta = PTHREAD_COND_INITIALIZER;
pthread_cond_t forno_pronto = PTHREAD_COND_INITIALIZER;

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
                // printf("\n entrou mutex e o valor do cont eh %d",cont_pizza);
                pthread_cond_signal(&novo_pedido);
			pthread_mutex_unlock(&contador);
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
    pthread_mutex_lock(&preparacao);
        // printf("Preparando massa");
        preparando = 1;
        sleep(2);
        pthread_cond_signal(&massa_pronta);
    pthread_mutex_unlock(&preparacao);
}

void montagem_pizza() {
    while(preparando < 1) {
        pthread_cond_wait(&massa_pronta,&preparacao);
    }
    pthread_mutex_lock(&preparacao);
        montando_pizza = 1;
        sleep(5);
        montando_pizza = -1;
        pthread_cond_signal(&montagem_pronta);
    pthread_mutex_unlock(&preparacao);
}

void coloca_forno() {
    while(preparando < 1 && montando_pizza < 0) {
        pthread_cond_wait(&montagem_pronta, &preparacao);
    }
    pthread_mutex_lock(&preparacao);
        sleep(5);
        preparacao = 0;
        montando_pizza = 0;
        pthread_cond_signal(&forno_pronto);
    pthread_mutex_unlock(&preparacao);
}

void empacota() {

}

void entrega_final() {}

void main() {
    printf("*Pizzaria Ana e Marina* \n");
    printf("Tecle novo para adicionar um pedido \n");
    pthread_create(&recebe, NULL, (void*)recebe_pedido, NULL);
    pthread_create(&massa, NULL, (void*)prepara_massa, NULL);

    pthread_join(recebe,NULL);
    pthread_join(massa,NULL);

}
