#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM 10

pthread_mutex_t m[NUM] ;
pthread_t worker_thread[NUM] ;

void *
worker_func (void * arg)
{
	int n = *((int *)arg) ;
	printf("worker %d started.\n", n) ;

	pthread_mutex_lock(&m[n%NUM]) ;
	sleep(1) ;
	pthread_mutex_lock(&m[(n+1)%NUM]) ;
	pthread_mutex_unlock(&m[(n+1)%NUM]) ;
	pthread_mutex_unlock(&m[n%NUM]) ;
	
	printf("worker %d finished.\n", n) ;
}

int
main ()
{
	int arg[NUM] = {} ;
	for (int i = 0; i < NUM; i++) {
		pthread_mutex_init(&m[i], 0x0) ;
		arg[i] = i ;
	}

	for (int i = 0; i < NUM; i++) {
		if (pthread_create(&worker_thread[i], 0x0, worker_func, &arg[i] ) != 0) {
			perror("pthread_create") ;
			exit(EXIT_FAILURE) ;
		}
	}

	for (int i = 0; i < NUM; i++)
		pthread_join(worker_thread[i], 0x0) ;
}
