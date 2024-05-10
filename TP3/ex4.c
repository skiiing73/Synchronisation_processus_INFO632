#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>

#define N 5 // Taille du tampon

char tampon[N][100]; // Déclaration du tampon
int in = 0; 
int out = 0; 
int k=0;
sem_t scons; 
sem_t sprod; 

void Produire(char *message) {
    sprintf(message, "Message: %d\n", k);
    k++;
    printf("Message produit: %s", message);
}

void Consommer(char *message) {
    int i = 0;
    while (message[i] != '\0') {
        message[i] = toupper(message[i]);
        i++;
    }
    printf("Message consommé: %s", message);
}

void *Production(void *arg) {
    int i;
    for (i = 0; i < 100; i++) {
        char message[100]; // Message produit par le producteur
        Produire(message); // Production du message
        sem_wait(&sprod); // Attends qu'une des cases du tampon soit libre 
        strcpy(tampon[in], message); // Stockage du message dans le tampon
        in = (in + 1) % N; 
        sem_post(&scons); // Notification du consommateur qu'un message est disponible dans le tampon
    }
    return NULL;
}

void *Consommation(void *arg) {
    int i;
    for (i = 0; i < 100; i++) {
        char message[100]; // Message consommé par le consommateur
        sem_wait(&scons); // Attend qu'une case du tampon soit pleine 
        strcpy(message, tampon[out]); // Récupération du message depuis le tampon
        out = (out + 1) % N; 
        sem_post(&sprod); // Notification du producteur qu'une case du tampon est libérée
        Consommer(message); // Consommation du message
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    sem_init(&scons, 0, 0); // Initialisé à 0 pour indiquer que le tampon est vide
    sem_init(&sprod, 0, N); // Initialisé à N pour indiquer que toutes les cases du tampon sont disponibles
    
    if (pthread_create(&thread1, NULL, Production, NULL) < 0) {
        perror("Error: thread cannot be created");
        exit(1);
    }
    if (pthread_create(&thread2, NULL, Consommation, NULL) < 0) {
        perror("Error: thread cannot be created");
        exit(1);
    }
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    sem_destroy(&scons);
    sem_destroy(&sprod);

    return 0;
}
