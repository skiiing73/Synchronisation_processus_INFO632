#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> 

/* Global variables */
int x = 0;
sem_t * sync_x;

/* Thread function */

void a() {
    sleep(10);
    printf("X = %d\n", x);
    x = 55;
}

void b() {
    printf("X = %d\n", x);
}

void *p1(void *arg) {
    a();
    sem_post(sync_x);
    return NULL;
}

void *p2(void *arg) {
    /* wait for the first thread */
    sem_wait(sync_x);
    b();
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    /* semaphore sync_x should be initialized by 0 */
    sync_x = sem_open("sync_x", O_CREAT, 0644, 0);
    
    if (pthread_create(&thread1, NULL, p1, NULL) < 0) {
        perror("Error: thread cannot be created");
        exit(1);
    }
    if (pthread_create(&thread2, NULL, p2, NULL) < 0) {
        perror("Error: thread cannot be created");
        exit(1);
    }
    /* wait for created thread to terminate */
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    /* destroy semaphore sync */
    sem_destroy(sync_x);
    exit(0);
}
