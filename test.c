#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *
worker (void * arg)
{
	printf("Hello\n") ;
	return 0 ; 
}

int
main ()
{
	pthread_t p ;
	pthread_create(&p, 0x0, worker, 0x0) ;
	pthread_join(p, 0x0) ;
	return 0 ;
}
