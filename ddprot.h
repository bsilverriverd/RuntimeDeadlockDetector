#ifndef DDPROT_H
#define DDPROT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#define UNLOCK 0x00000000
#define LOCK 0x00000001
#define BT_BUF_SIZE 2 

int
read_bytes (int fd, void * a, int len)
{
	char * s = (char *) a ;
	
	int i ;
	for (i = 0 ; i < len ; ) {
		int b ;
		b = read(fd, s + i, len - i) ;
		if (b == 0)
			break ;
		i += b ;
	}
	return i ; 
}

int 
write_bytes (int fd, void * a, int len)
{
	char * s = (char *) a ;

	int i = 0 ; 
	while (i < len) {
		int b ;
		b = write(fd, s + i, len - i) ;
		if (b == 0)
			break ;
		i += b ;
	}
	return i ;	
}

int
ddread (int fd, int * mode, pthread_t * tid, pthread_mutex_t ** mutex, long int * addr)
{
/*
	if (mkfifo(".ddtrace", 0666)) {
		if (errno != EEXIST) {
			perror("mkfifo") ;
			exit(EXIT_FAILURE) ;
		}
	}

	int fd = open(".ddtrace", O_RDONLY | O_SYNC) ;
	if (fd < 0) {
		perror("open") ;
		exit(EXIT_FAILURE) ;
	}
*/
//	flock(fd, LOCK_EX) ;
		if (read_bytes(fd, mode, sizeof(int)) != sizeof(int)) {
//			flock(fd, LOCK_UN) ;
//			perror("read1") ;
			return 0;
		}
		if (read_bytes(fd, tid, sizeof(pthread_t)) != sizeof(pthread_t)) {
//			flock(fd, LOCK_UN) ;
//			perror("read2") ;
			return 0 ;
		}
		if (read_bytes(fd, mutex, sizeof(pthread_mutex_t *)) != sizeof(pthread_mutex_t *)) {
//			flock(fd, LOCK_UN) ;
			perror("read3") ;
			return 0 ;
		}
		if (read_bytes(fd, addr, sizeof(long int)) != sizeof(long int)) {
//			flock(fd, LOCK_UN) ;
			perror("read3") ;
			return 0 ;
		}


//	close(fd) ;
	return 1 ;
}

void
ddwrite (int * mode, pthread_t * tid, pthread_mutex_t * mutex, long int * addr)
{
	if (mkfifo(".ddtrace", 0666)) {
		if (errno != EEXIST) {
			perror("mkfifo") ;
			exit(EXIT_FAILURE) ;
		}
	}

	printf("[WRITE] %d %ld %p\n", *mode, *tid, mutex) ;
	int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;
	if (fd < 0) {
		perror("open") ;
		exit (EXIT_FAILURE) ;
	}	
	write_bytes(fd, mode, sizeof(int)) ;
	write_bytes(fd, tid, sizeof(pthread_t)) ;
	write_bytes(fd, &mutex, sizeof(pthread_mutex_t *)) ;
	write_bytes(fd, addr, sizeof(long int)) ;
	close(fd) ;
}
#endif
