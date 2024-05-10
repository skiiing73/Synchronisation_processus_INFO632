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
    int tours_fini;//permet de savoir si un tour est en cours
    int position;
    float tour_time;//temps pour chaque tour
    float total_time;//temps total de la course
    float avg_time;//temps moyen au tour 
    pthread_t thread;
    pthread_mutex_t mutex; //creation d'un mutex pour chaque voiture
} Voiture;

Voiture Liste_voitures[NUM_CARS];//creation d'une liste de voiture

int ordre_arrivee[NUM_CARS];//creation d'une liste pour enregistrer le classement final

pthread_t arbitre;

pthread_barrier_t barriere;//creation d'une barriere 

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//initialisation d'un mutex 

int position_course=0;//variable position course qui permet de savoir la position de chaque voiture
int nb_voitures_arrivees=0;//nombre de voitures ayant fini


void * course_voiture(void *idvoiture) {//fonction définissant ce que doit faire la voiture, recoit un pointeur vers une voiture en tant qu'argument

    Voiture *voiture = (Voiture *)idvoiture;    
    
    pthread_barrier_wait(&barriere); // Attente du signal de départ de l'arbitre


    while (voiture->tours_completes < NUM_LAPS) {//tant que la voiture n'a pas effectué tous ses tours
        float tour_time = 0.5 + ((float)rand() / RAND_MAX) * 1.0;
        usleep((int)(tour_time * 1000000)); // Convertir le temps en microsecondes et attendre ce temps
        voiture->tour_time=tour_time;//ajoute le temps au tour a la voiture
        voiture->total_time += tour_time;//ajoute le temps au tour au temps total de la voiture

        pthread_mutex_lock(&voiture->mutex);
        voiture-> tours_fini++;// Signal pour indiquer la fin du tour
        voiture-> tours_completes++;
        pthread_mutex_unlock(&voiture->mutex);
        
    }
    
    
    pthread_exit(NULL);
}

void *arbitre_role() {
    pthread_barrier_wait(&barriere); // Signal pour libérer toutes les voitures simultanément

    printf("L'arbitre déclenche le départ de la course\n");

    while (1) { // Boucle infinie
        for (int i = 0; i < NUM_CARS; i++) {//pour chaque voiture
            
            if (Liste_voitures[i].tours_fini == 1) {//si la voiture a finit son tour en cours

                //nous arrivons dans la section critique
                //nous empechons donc la voiture de continuer son programme tant qe l'arbitre n'a pas noté les temps et les tours de la voiture
                pthread_mutex_lock(&Liste_voitures[i].mutex);
                printf("Voiture%d a fini le tour %d en %f secondes\n", Liste_voitures[i].id, Liste_voitures[i].tours_completes, Liste_voitures[i].tour_time);
                if (Liste_voitures[i].tours_completes == NUM_LAPS) {//si la voiture a fini la course

                    //section critique partagée par toutes les voitures pour la fin de course
                    pthread_mutex_lock(&mutex);
                    position_course++;
                    Liste_voitures[i].position = position_course;
                    ordre_arrivee[Liste_voitures[i].position - 1] = Liste_voitures[i].id;
                    printf("Voiture%d a fini la course!\n", Liste_voitures[i].id);//affichage des informations
                    nb_voitures_arrivees++;
                    pthread_mutex_unlock(&mutex);
                    Liste_voitures[i].avg_time=Liste_voitures[i].total_time/NUM_LAPS;
                }
            
                Liste_voitures[i].tours_fini = 0;//on indique que l'on repart dans un nouveau tour 
                pthread_mutex_unlock(&Liste_voitures[i].mutex);//on laisse la voiture refaire un tour
                
            }
            
        } 
        if (nb_voitures_arrivees == NUM_CARS) // Si toutes les voitures ont terminé la course, sortir de la boucle
            break;
    }

    printf("Course terminée voici le classement:\n");
    for (int i = 0; i < NUM_CARS; i++) {
        int idvoiture=ordre_arrivee[i];
        printf("%d. Voiture %d en %f, secondes avec une moyenne de %f s/tour\n", i + 1, idvoiture,Liste_voitures[idvoiture].total_time,Liste_voitures[idvoiture].avg_time);
    }

    pthread_exit(NULL);
}


int main() {
    srand(time(NULL)); // Initialisation du générateur de nombres aléatoires
    pthread_barrier_init(&barriere, NULL, NUM_CARS + 1);
    //on initialise la barriere a NUM_CARS + 1, car nous avons besoin 
    //que l'arbitre et toutes les voitures atteignent ce point avant de continuer.

    pthread_create(&arbitre, NULL, arbitre_role, NULL);

    for (int i = 0; i < NUM_CARS; i++) {//création du nombre de voitures et de leurs threads correspondant
        Liste_voitures[i].id = i;
        Liste_voitures[i].tours_completes=0;
        pthread_mutex_init(&Liste_voitures[i].mutex, NULL);
        pthread_create(&Liste_voitures[i].thread, NULL, course_voiture, (void *)&Liste_voitures[i]);
    }

    pthread_join(arbitre, NULL);

    return 0;
}
