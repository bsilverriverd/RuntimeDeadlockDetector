#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "ddprot.h"

int
main ()
{
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
	while (1) {
		int mode ;
		pthread_t p ;
		pthread_mutex_t * mutex ;
		flock(fd, LOCK_EX) ;
			int ret = ddread(fd, &mode, &p, &mutex) ;
		flock(fd, LOCK_UN) ;
	
		if (ret)
			printf("[READ] %d %ld %p\n", mode, p, mutex) ;
	}	
	close(fd) ;
}
