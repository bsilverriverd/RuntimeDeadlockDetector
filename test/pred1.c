#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
void* myfunc(){
    pthread_mutex_lock(&m1);
    pthread_mutex_lock(&m2);
    printf("myfunc\n");
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);
    pthread_mutex_lock(&m2);
    pthread_mutex_lock(&m1);
    printf("myfunc\n");
    pthread_mutex_unlock(&m1);
    pthread_mutex_unlock(&m2);
}
void main(){
    pthread_t thread1;
    pthread_create(&thread1, NULL, myfunc, NULL);
    pthread_join(thread1, NULL);
    printf("end\n");
}
