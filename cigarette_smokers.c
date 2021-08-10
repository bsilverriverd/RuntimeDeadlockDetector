#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int item[3] = { 0, 0, 0 } ;
pthread_mutex_t item_lock[3] ;
pthread_cond_t item_cond[3] ;
int agent = 0 ;
pthread_mutex_t agent_lock ;
pthread_cond_t agent_cond ;
pthread_t smoker_thread[3] ;
pthread_mutex_t stdio_lock ;

const char s[3][8] = {
	"TOBACCO",
	"MATCH",
	"PAPER"
} ;

void *
smoker_func (void * arg)
{
	int * own = (int *)arg ;
	while (1) {
		for (int i = 0; i < 3; i++) {
			if (i == *own)
				continue ;
			pthread_mutex_lock(&item_lock[i]) ;
				pthread_mutex_lock(&stdio_lock) ;
					printf("smoker with %s needs %s.\n", s[*own], s[i]) ;
				pthread_mutex_unlock(&stdio_lock) ;
				while (!item[i])
					pthread_cond_wait(&item_cond[i], &item_lock[i]) ;
				--item[i] ;
				pthread_mutex_lock(&stdio_lock) ;
					printf("smoker with %s acquired %s.\n", s[*own], s[i]) ;
				pthread_mutex_unlock(&stdio_lock) ;
			pthread_mutex_unlock(&item_lock[i]) ;
		}
		pthread_mutex_lock(&stdio_lock) ;
			printf("smoker with %s is smoking.\n", s[*own]) ;
		pthread_mutex_unlock(&stdio_lock) ;
		sleep(3) ;
		pthread_mutex_lock(&stdio_lock) ;
			printf("smoker with %s is done smoking.\n", s[*own]) ;
		pthread_mutex_unlock(&stdio_lock) ;

		pthread_mutex_unlock(&agent_lock) ;
			++agent ;
			pthread_cond_signal(&agent_cond) ;
		pthread_mutex_unlock(&agent_lock) ;
	}
}

int
main ()
{
	// initializing locks, conditional variables
	pthread_mutex_init(&stdio_lock, 0x0) ;
	pthread_mutex_init(&agent_lock, 0x0) ;
	pthread_cond_init(&agent_cond, 0x0) ;
	for (int i = 0; i < 3; i++) {
		pthread_mutex_init(&item_lock[i], 0x0) ;
		pthread_cond_init(&item_cond[i], 0x0) ;
	} 
	srand(time(0x0)) ;

	int arg[3] = { 0, 1, 2 } ;
	for (int i = 0; i < 3; i++) {
		pthread_create(&smoker_thread[i], 0x0, smoker_func, &arg[i]) ;
	}

	// agent loop
	while (1) {
		int num = rand() % 3 ;
		pthread_mutex_lock(&stdio_lock) ;
			printf("This round without %s.\n", s[num]) ;
		pthread_mutex_unlock(&stdio_lock) ;

		for (int i = 0; i < 3; i++) {
			if (i == num) 
				continue ;
			pthread_mutex_lock(&item_lock[i]) ;
				++item[i] ;
				pthread_cond_signal(&item_cond[i]) ;
			pthread_mutex_unlock(&item_lock[i]) ;
		}
		pthread_mutex_lock(&agent_lock) ;
			while (!agent)
				pthread_cond_wait(&agent_cond, &agent_lock) ;
			pthread_mutex_lock(&stdio_lock) ;
				printf("----------------------------\n") ;
			pthread_mutex_unlock(&stdio_lock) ;
			--agent ;
		pthread_mutex_unlock(&agent_lock) ;
	}

	for (int i = 0; i < 3; i++) {
		pthread_join(smoker_thread[i], 0x0) ;
	}
}
