#include <stdio.h>
#include <pthread.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER ;

void
huh (pthread_mutex_t * m)
{
	printf("%p\n", m) ;
}

void
hey (pthread_mutex_t * m)
{
	printf("%p\n", m) ;
	huh(m) ;
}

int
main ()
{
	printf("%p\n", &m) ;
	printf("%zu\n", sizeof(&m)) ;
	printf("%ld\n", pthread_self()) ;
	printf("%ld\n", sizeof(pthread_t)) ;
	hey(&m) ;

	printf("%zu\n", sizeof(int *)) ;
	printf("%zu\n", sizeof(long int *)) ;
	printf("%zu\n", sizeof(long long int *)) ;
	printf("%zu\n", sizeof(char *)) ;
	printf("%zu\n", sizeof(pthread_t *)) ;
	printf("%zu\n", sizeof(void *)) ;
	printf("%zu\n", sizeof(void **)) ;
}
