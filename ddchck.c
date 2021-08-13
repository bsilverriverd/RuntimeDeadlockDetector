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
#include "ddgraph.h"

int
main (int argc, char * argv[])
{
	if (argc != 2) {
		printf("Few arguements!\n") ;
		exit(EXIT_SUCCESS) ;
	}
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

	graph * lockgraph = graph_init() ;
	if (lockgraph == 0x0) {
		perror("graph_init()") ;
		exit(EXIT_FAILURE) ;
	}

	while (1) {
		int mode ;
		pthread_t tid ;
		pthread_mutex_t * mutex ;
		long int addr ;
		flock(fd, LOCK_EX) ;
			int ret = ddread(fd, &mode, &tid, &mutex, &addr) ;
		flock(fd, LOCK_UN) ;

		if (!ret)
			continue ;
#ifdef DEBUG
	fprintf(stderr, "[READ] %d %ld %p\n", mode, tid, mutex) ;
#endif
		if (mode == LOCK) {
			if (!graph_lock(lockgraph, tid, mutex)) {
				detected(lockgraph, argv[1], addr) ;
				break ;
			}
		} else if (mode == UNLOCK) {
			graph_unlock(lockgraph, tid, mutex) ;
		} else {
			continue ;
		}
#ifdef DEBUG
	graph_print(lockgraph) ;
#endif
		if (graph_detect(lockgraph)) {
			detected(lockgraph, argv[1], addr) ;
			break ;
		}
	}	
	close(fd) ;
} /* main */
