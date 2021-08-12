#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
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
	pthread_t tid = pthread_self() ;
	
	void *buffer[BT_BUF_SIZE] ;
	int nptrs = backtrace(buffer, BT_BUF_SIZE) ;
	char ** strings = backtrace_symbols(buffer, nptrs) ;
	long int addr = 0 ;
	for (int i = 0; i < nptrs; i++) {
#ifdef DEBUG
	fprintf(stderr, "[DEBUG] %s\n", strings[i]) ;
#endif
		char * tok = 0x0 ;
		tok = strtok(strings[i], "()") ;
		tok = strtok(0x0, "()") ;
		addr = strtol(tok, 0x0, 16) ;
#ifdef DEBUG
	fprintf(stderr, "[DEBUG] %s %lx\n", strings[i], addr) ;
#endif
	}
	pthread_mutex_lock_p(&fifo_lock) ;
		ddwrite(&mode, &tid, mutex, &addr) ;
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
	pthread_t tid = pthread_self() ;
	void *buffer[BT_BUF_SIZE] ;
	int nptrs = backtrace(buffer, BT_BUF_SIZE) ;
	char ** strings = backtrace_symbols(buffer, nptrs) ;
	long int addr = 0 ;
	for (int i = 0; i < nptrs; i++) {
#ifdef DEBUG
	fprintf(stderr, "[DEBUG] %s\n", strings[i]) ;
#endif
		char * tok = 0x0 ;
		tok = strtok(strings[i], "()") ;
		tok = strtok(0x0, "()") ;
		addr = strtol(tok, 0x0, 16) ;
#ifdef DEBUG
	fprintf(stderr, "[DEBUG] %s %lx\n", strings[i], addr) ;
#endif
	}
	pthread_mutex_lock_p(&fifo_lock) ;
		ddwrite(&mode, &tid, mutex, &addr) ;
	pthread_mutex_unlock_p(&fifo_lock) ;

	return pthread_mutex_unlock_p(mutex) ;
}
