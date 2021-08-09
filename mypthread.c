#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>
#include <execinfo.h>
int
pthread_create (pthread_t * restrict thread, 
				const pthread_attr_t * restrict attr,
				void * (*start_routine)(void *),
				void * restrict arg)
{
	int (*pthread_create_p)(pthread_t * restrict thread,
								const pthread_attr_t * restrict attr,
								void * (*start_routine)(void *),
								void * restrict arg) ;
	char * error ;
	pthread_create_p = dlsym(RTLD_NEXT, "pthread_create") ; 
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	fprintf(stderr, "Hello Pthread!\n") ;
	return pthread_create_p(thread, attr, start_routine, arg) ;
}
