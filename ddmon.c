#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>

#include "ddprot.h"

pthread_mutex_t fifo_lock = PTHREAD_MUTEX_INITIALIZER ;

int
pthread_mutex_lock (pthread_mutex_t * mutex)
{
	int (*pthread_mutex_lock_p)(pthread_mutex_t * mutex) ;
	int (*pthread_mutex_unlock_p)(pthread_mutex_t * mutex) ;
	char * error ;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;

	int mode = LOCK ;
	pthread_t p = pthread_self() ;
	pthread_mutex_lock_p(&fifo_lock) ;
		ddwrite(&mode, &p, mutex) ;
	pthread_mutex_unlock_p(&fifo_lock) ;

	return pthread_mutex_lock_p(mutex) ;
}

int
pthread_mutex_unlock (pthread_mutex_t * mutex)
{
	int (*pthread_mutex_lock_p)(pthread_mutex_t * mutex) ;
	int (*pthread_mutex_unlock_p)(pthread_mutex_t * mutex) ;
	char * error ;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	
	int mode = UNLOCK ;
	pthread_t p = pthread_self() ;
	pthread_mutex_lock_p(&fifo_lock) ;
		ddwrite(&mode, &p, mutex) ;
	pthread_mutex_unlock_p(&fifo_lock) ;

	return pthread_mutex_unlock_p(mutex) ;
}
