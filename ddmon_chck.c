#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>

#include "ddgraph.h"

#define BT_BUF_SIZE 2

graph * lockgraph = 0x0 ;
static volatile int init = 0 ;

int
pthread_mutex_lock (pthread_mutex_t * mutex)
{
	if (!init) {
		init = 1 ;
		lockgraph = graph_init() ;
		printf("init\n") ;
	}
	int (*pthread_mutex_lock_p)(pthread_mutex_t * mutex) ;
	char * error ;

	pthread_mutex_lock_p = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;

	pthread_t tid = pthread_self() ;
	
	void *buffer[BT_BUF_SIZE] ;
	int nptrs = backtrace(buffer, BT_BUF_SIZE) ;
	char ** strings = backtrace_symbols(buffer, nptrs) ;
	char * fname ;
	long int addr = 0 ;
	for (int i = 0; i < nptrs; i++) {
#ifdef DEBUG
	fprintf(stderr, "[DEBUG] %s\n", strings[i]) ;
#endif
		char * tok = 0x0 ;
		fname = strtok(strings[i], "(") ;
		tok = strtok(0x0, ")") ;
		addr = strtol(tok, 0x0, 16) ;
#ifdef DEBUG
	fprintf(stderr, "[DEBUG] %s %lx\n", strings[i], addr) ;
#endif
	}
	
	graph_lock(lockgraph, tid, mutex) ;
	if (graph_detect(lockgraph))
		detected(lockgraph, fname, addr) ;

	return pthread_mutex_lock_p(mutex) ;
}

int
pthread_mutex_unlock (pthread_mutex_t * mutex)
{
	if (!init) {
		init = 1 ;
		lockgraph = graph_init() ;
		printf("init\n") ;
	}
	int (*pthread_mutex_unlock_p)(pthread_mutex_t * mutex) ;
	char * error ;

	pthread_mutex_unlock_p = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	
	pthread_t tid = pthread_self() ;
	void *buffer[BT_BUF_SIZE] ;
	int nptrs = backtrace(buffer, BT_BUF_SIZE) ;
	char ** strings = backtrace_symbols(buffer, nptrs) ;
	char * fname = 0x0 ;
	long int addr = 0 ;
	for (int i = 0; i < nptrs; i++) {
#ifdef DEBUG
	fprintf(stderr, "[DEBUG] %s\n", strings[i]) ;
#endif
		char * tok = 0x0 ;
		fname = strtok(strings[i], "(") ;
		tok = strtok(0x0, ")") ;
		addr = strtol(tok, 0x0, 16) ;
#ifdef DEBUG
	fprintf(stderr, "[DEBUG] %s %lx\n", strings[i], addr) ;
#endif
	}
	
	graph_unlock(lockgraph, tid, mutex) ;
	if (graph_detect(lockgraph))
		detected(lockgraph, fname, addr) ;

	return pthread_mutex_unlock_p(mutex) ;
}
