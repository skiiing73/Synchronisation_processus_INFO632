#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> 

/* Global variables */
int x = 0;
sem_t * m;

/* Thread function */
void *thread(int i)
{
/* critical section */
    sem_wait(m);
    x = x + 1;
    sem_post(m);
    printf("x:%d, i:%d\n",i,x);
    return NULL;
}

void main ()
{
    pthread_t tid[10];
    int i;

    /* init sem at "1" */
    m = sem_open("/mutex", O_CREAT, 0644, 1);

    /* create TEN threads */
    for (i=0; i<10; i++)
    {
        if (pthread_create(&tid[i], NULL, thread, i) < 0) {
        perror("Error: thread cannot be created");
        exit(1);
    }
    }
    /* wait for all created thread to terminate */
    for (i=0; i<10; i++) pthread_join(tid[i], NULL);
        printf("Final value of x is %d\n", x);
    exit(0);
}
