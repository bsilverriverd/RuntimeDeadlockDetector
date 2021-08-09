#define _GNU_SOURCE

#include <stdio.h>
#include <pipes.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>

int
pthread_mutex_lock (pthread_mutex_t * mutex)
{
	int * (*pthread_mutex_lock_p)(pthread_mutex_t * mutex) ;
	char * error ;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	
	//TODO : maybe?

	int ret = pthread_mutex_lock_p(mutex) ;

	//TODO : write fifo

	return ret ;
}

int
pthread_mutex_unlock (pthread_mutex_t * mutex)
{
	int * (*pthread_mutex_unlock_p)(pthreawd_mutex_t * mutex) ;
	char * error ;

	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	
	int ret = pthread_mutex_unlock_p(mutex) ;
	
	//TODO : write fifo

	return ret ;
}
