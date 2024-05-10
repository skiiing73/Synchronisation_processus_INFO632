#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_CARS 3
#define NUM_LAPS 5

//Définition de la structure d'une voiture qui comprend son id, le nombre de tours complétés, sa position finale et son thread
typedef struct {
    int id;
    int tours_completes;
    int position;
    pthread_t thread;
} Voiture;

Voiture Liste_voitures[NUM_CARS];//creation d'une liste de voiture

int ordre_arrivee[NUM_CARS];//creation d'une liste pour enregistrer le classement final

pthread_t arbitre;

pthread_barrier_t barriere;//creation d'une barriere 

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//initialisation d'un mutex 

int position_course=0;//variable position course qui permet de savoir combien de voitures ont fini 


void *course_voiture(void *arg) {//fonction définissant ce que doit faire la voiture, recoit un pointeur vers une voiture en tant qu'argument

    Voiture *voiture = (Voiture *)arg;
    srand(time(NULL) + voiture->id); // Utilisation de l'identifiant pour initialiser le générateur de nombres aléatoires
    
    
    pthread_barrier_wait(&barriere); // Attente du signal de départ de l'arbitre


    while (voiture->tours_completes < NUM_LAPS) {//tant que la voiture n'a pas effectué tous ses tours

        usleep(rand() % 1000000); // Simuler le temps de tour
        printf("Voiture %d a fini le tour  %d\n", voiture->id, voiture->tours_completes + 1);
        voiture->tours_completes++;

    }

    //nous ne voulons pas que plusieurs voitures finissent en meme temps
    //il est donc essentiel de protéger l'arrivée par un mutex pour qu'une seule voiture a la fois y ait accès
    pthread_mutex_lock(&mutex);
    position_course++;//une voiture est arrivée    
    voiture->position =position_course;//on attribut la position d'arrivée a la voiture
    ordre_arrivee[voiture->position - 1] = voiture->id; // Enregistrement de l'ordre d'arrivée dans la liste d'arrivée
    printf("Voiture %d a fini la course!\n", voiture->id);
    pthread_mutex_unlock(&mutex);
    
    pthread_exit(NULL);
}

void *arbitre_role() {//fonction définissant le rôle de l'arbitre

    pthread_barrier_wait(&barriere); // Signal pour libérer toutes les voitures simultanément

    printf("L'arbitre déclenche le départ de la course\n");

    for (int i = 0; i < NUM_CARS; i++) {//lancement de tous les threads
        pthread_join(Liste_voitures[i].thread, NULL);
    }

    printf("Course terminée voici le classement:\n");
    for (int i = 0; i < NUM_CARS; i++) {//affichage du classement d'arrivée en parcourant la liste d'arrivée
        printf("%d. Voiture %d\n", i + 1, ordre_arrivee[i]);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_barrier_init(&barriere, NULL, NUM_CARS + 1);
    //on initialise la barriere a NUM_CARS + 1, car nous avons besoin 
    //que l'arbitre et toutes les voitures atteignent ce point avant de continuer.

    pthread_create(&arbitre, NULL, arbitre_role, NULL);

    for (int i = 0; i < NUM_CARS; i++) {//création du nombre de voitures et de leurs threads correspondant
        Liste_voitures[i].id = i;
        pthread_create(&Liste_voitures[i].thread, NULL, course_voiture, (void *)&Liste_voitures[i]);
    }

    pthread_join(arbitre, NULL);

    return 0;
}
