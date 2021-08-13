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
graph_predict (graph * g)
{
	edge * itr = g->elist ;
	while (itr) {
		itr->visited = 0 ;
		itr = itr->next ;
	}
	itr = g->elist ;
	int visit = 1 ;
	while (itr) {
		if (!itr->visited) {
			itr->visited = visit ;
			node * next = itr->v ;
			edge * curr = g->elist ;
			while (curr) {
				if (!pthread_equal(curr->u->tid, next->tid)) {
					if (curr->u->m == next->m) {
						if (curr->visited == visit) {
							return 1 ;
						} else {
							curr->visited = visit ;
							next = curr->v ;
							curr = g->elist ;
						}
					} else {
						curr = curr->next ;
					}
				} else {
					curr = curr->next ;
				}
			} // while(curr)
		}
		itr = itr->next ;
		++visit ;
	} // while(itr) 
	return 0 ;
}

void
predicted (graph * g, char * fname, long int addr)
{
	graph_print(g) ;
	printf("WARNING!\n") ;
		
	char command[1024] ;
	char buf[1024] ;
	snprintf(command, 1024, "addr2line -f -e %s %lx", fname, addr) ;
	FILE * fp = popen(command, "r") ;
	while (fgets(buf, 1024, fp) != 0x0)
		printf("%s", buf) ;
	pclose(fp) ;
} /* detected */

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
			//graph_unlock(lockgraph, tid, mutex) ;
			node_delete(&lockgraph->nlist, tid, mutex) ;
		} else {
			continue ;
		}
#ifdef DEBUG
	graph_print(lockgraph) ;
#endif
		if (graph_predict(lockgraph)) {
			predicted(lockgraph, argv[1], addr) ;
			break ;
		}
	}	
	close(fd) ;
} /* main */
