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

#define BUF_SIZE 5

int cont_pizza = 0;
int cont_prontas = 0;
int fim = 0;
int preparando = 0;
int montando_pizza = 0;
int pacotes = 10;
int pizza_pronta = 0;
int contador_vezes = 0;
int entregador_ocupado = 0;
int pizzas_a_entregar = 0;


char comando[20];

pthread_t recebe, massa, montagem, forno, empacota, entrega, show;

pthread_mutex_t contador = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t comando_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t preparacao = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t montagem_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t forno_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t empacota_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tela = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t entregador_fora = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t novo_pedido = PTHREAD_COND_INITIALIZER;
pthread_cond_t massa_pronta = PTHREAD_COND_INITIALIZER;
pthread_cond_t montagem_pronta = PTHREAD_COND_INITIALIZER;
pthread_cond_t forno_pronto = PTHREAD_COND_INITIALIZER;
pthread_cond_t pedido_pronto = PTHREAD_COND_INITIALIZER;

void limpa_linha() {
    scanf("%*[^\n]");
    scanf("%*c");
}

 void recebe_pedido(){
    char c;

    while(1) {
    scanf("%c",&c);
    pthread_mutex_lock(&contador);
        cont_pizza= 1;
        contador_vezes++;
    pthread_mutex_unlock(&contador);
    
    pthread_cond_signal(&novo_pedido);
    
    limpa_linha();
    }
} 

void prepara_massa() {
    while(1) {

    pthread_mutex_lock(&contador);

    while(contador_vezes < 1) {
        pthread_cond_wait(&novo_pedido,&contador);
        }

    pthread_mutex_unlock(&contador);
    pthread_mutex_lock(&preparacao);
    preparando = 1;
    cont_pizza = 0;
    pthread_mutex_unlock(&preparacao);
        
    sleep(1);
    pthread_cond_signal(&massa_pronta);

    }
}

void montagem_pizza() {

while(1){
    pthread_mutex_lock(&preparacao);
    while(preparando < 1) {
        pthread_cond_wait(&massa_pronta,&preparacao);
        }
    pthread_mutex_unlock(&preparacao);

    pthread_mutex_lock(&montagem_mutex);
    montando_pizza = 1;
    preparando = 0;
    pthread_mutex_unlock(&montagem_mutex);
    sleep(3);
    pthread_cond_signal(&montagem_pronta);
}
}

void coloca_forno() {
    while(1){

    pthread_mutex_lock(&montagem_mutex);
     while(montando_pizza < 1) {
        pthread_cond_wait(&montagem_pronta,&montagem_mutex);
    }
    pthread_mutex_unlock(&montagem_mutex);
    pthread_mutex_lock(&forno_mutex);
        pizza_pronta = 1;
        montando_pizza = 0;
        sleep(2);
    pthread_mutex_unlock(&forno_mutex);
    
    pthread_cond_signal(&forno_pronto);
}
}

void empacotando() {
        while(1){

    pthread_mutex_lock(&forno_mutex);
    while(pizza_pronta < 1) {
        pthread_cond_wait(&forno_pronto,&forno_mutex);
    }
    pthread_mutex_unlock(&forno_mutex);
    sleep(1);
    pthread_mutex_lock(&contador);
    contador_vezes--;
    pthread_mutex_unlock(&contador);
    pthread_mutex_lock(&empacota_mutex);
        pizza_pronta=0;
        cont_prontas= cont_prontas+1;
        pizzas_a_entregar++;
        pacotes = pacotes-1;
        pthread_cond_signal(&pedido_pronto);
    pthread_mutex_unlock(&empacota_mutex);
    }
}

void mostra_tela () {
    while(1) {
        pthread_mutex_lock(&tela);
        system("clear");
	    printf("*-*-*-*-  Pizzaria da Ana e da Marina    *-*-*-*-\n\n");
        if (contador_vezes > 0) {
            printf("\nA pizza esta sendo preparada\n");
        } else {
            printf("\nSem pizzas no processo\n");
        }
        printf("No momento temos %d pedidos \n",contador_vezes);
        printf("Ja preparamos %d pedidos \n",cont_prontas);
        printf("Ainda temos %d pacotes \n",pacotes);
        if (pacotes < 1) {
            printf("\nPrecisa comprar pacotes\n");
        }
        if (pacotes == 0) {
            system("clear");
            printf("\nSem pacotes\n");
            break;
        }
        if (entregador_ocupado > 0) {
            printf("\nEntregador saiu\n");
        } else {
            printf("\nEntregador na espera\n");
        }
        printf("Entre com um pedido(caracter): \n");  
        pthread_mutex_unlock(&tela);
        sleep(5);
    }

}

void entrega_final() {
    while(1) {
    pthread_mutex_lock(&empacota_mutex);
    while (pizzas_a_entregar < BUF_SIZE) {
        pthread_cond_wait(&pedido_pronto,&empacota_mutex);
    }
    pthread_mutex_unlock(&empacota_mutex);

    pthread_mutex_lock(&entregador_fora);
    entregador_ocupado = 1;
    pizzas_a_entregar=0;
    pthread_mutex_unlock(&entregador_fora);
    sleep(10);

    pthread_mutex_lock(&entregador_fora);
    entregador_ocupado = 0;
    pthread_mutex_unlock(&entregador_fora);
    }
}




void main() {
    printf("*Pizzaria Ana e Marina* \n");
    pthread_create(&recebe, NULL, (void*)recebe_pedido, NULL);
    pthread_create(&massa, NULL, (void*)prepara_massa, NULL);
    pthread_create(&montagem, NULL, (void*)montagem_pizza, NULL);
    pthread_create(&forno, NULL, (void*)coloca_forno, NULL);
    pthread_create(&empacota, NULL, (void*)empacotando, NULL);
    pthread_create(&show, NULL, (void*)mostra_tela, NULL);
    pthread_create(&entrega, NULL, (void*)entrega_final, NULL);

    pthread_join(recebe,NULL);
    pthread_join(massa,NULL);
    pthread_join(montagem,NULL);
    pthread_join(forno,NULL);
    pthread_join(empacota,NULL);
    pthread_join(show,NULL);
    pthread_join(entrega,NULL);

}
