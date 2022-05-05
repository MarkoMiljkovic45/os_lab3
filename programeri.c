#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#define LINUX_PROGRAMER_COUNT 5
#define MS_PROGRAMER_COUNT 10
#define PROGRAMER_TYPE_COUNT 2
#define EATING_DURATION_IN_SECS 1

int programer_count[PROGRAMER_TYPE_COUNT];
int resturant_queue[PROGRAMER_TYPE_COUNT];
int entered_count[PROGRAMER_TYPE_COUNT];

int starvation_limit = LINUX_PROGRAMER_COUNT < MS_PROGRAMER_COUNT ? LINUX_PROGRAMER_COUNT : MS_PROGRAMER_COUNT; //This limit is used to fix the starvation problem

pthread_mutex_t monitor;
pthread_cond_t cond_queue[PROGRAMER_TYPE_COUNT];

void udji(int vrsta) {
    pthread_mutex_lock(&monitor);
    resturant_queue[vrsta]++;                               //Enter the resturant queue
    while (programer_count[1 - vrsta] > 0 ||                //Wait until there are no more programers of the opposite type in the resutrant OR
           entered_count[1 - vrsta] < starvation_limit) {   //the number of programers of the opposite type that entered is lower than the starvation limit
              
        pthread_cond_wait(&cond_queue[vrsta], &monitor);
    }

    char programer_type[] = vrsta == 0 ? "Linux" : "Microsoft";
    printf("%s programer entered the resturant\n", programer_type);

    programer_count[vrsta]++;                               //Increment the number of programers in the resturan of a certain type
    resturant_queue[vrsta]--;                               //Leave the resturant queue

    if (resturant_queue[1 - vrsta] > 0)                     //If there are programers of the opposite type waitng, start counting the number
        entered_count[vrsta]++;                             //of programers of my type that entered the resturant (starvation limit)

    pthread_mutex_unlock(&monitor);
}

void obavi(void) {
    sleep(EATING_DURATION_IN_SECS);
}

void izadji(int vrsta) {
    pthread_mutex_lock(&monitor);
    programer_count[vrsta]--;                               //Decrement the number of programers in the resturan of a certain type

    char programer_type[] = vrsta == 0 ? "Linux" : "Microsoft";
    printf("%s programer left the resturant\n", programer_type);

    if (programer_count[vrsta] == 0) {                      //
        entered_count[1 - vrsta] = 0;
        pthread_cond_broadcast(&cond_queue[1 - vrsta]);
    }
    pthread_mutex_unlock(&monitor);
}

void *programer(void *vrsta) {
    udji(*((int *) vrsta));
    obavi();
    izadji(*((int *) vrsta));
}


int main(void) {

    pthread_t linux_thread_ids[LINUX_PROGRAMER_COUNT];
    pthread_t ms_thread_ids[MS_PROGRAMER_COUNT];

    int linux_index = 0;
    int ms_index = 1;
    
    //Monitor init
    pthread_mutex_init(&monitor, NULL);

    //Condition queue init
    for (int i = 0; i < PROGRAMER_TYPE_COUNT; i++) {
        pthread_cond_init(&cond_queue[i], NULL);
    }

    //Programer threads init
    for (int i = 0; i < LINUX_PROGRAMER_COUNT; i++) {
        if (pthread_create(&linux_thread_ids[i], NULL, programer, &linux_index) != 0) {
            printf("Failed to create thread!\n");
            exit(1);
        }
    }

    for (int i = 0; i < MS_PROGRAMER_COUNT; i++) {
        if (pthread_create(&ms_thread_ids[i], NULL, programer, &ms_index) != 0) {
            printf("Failed to create thread!\n");
            exit(1);
        }
    }

    //Wait for threads to finish
    for (int i = 0; i < LINUX_PROGRAMER_COUNT; i++)
        pthread_join(linux_thread_ids[i], NULL);

    for (int i = 0; i < MS_PROGRAMER_COUNT; i++)
        pthread_join(ms_thread_ids[i], NULL);
    
    return 0;
}