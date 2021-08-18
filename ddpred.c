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
#include "ddpredgraph.h"

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
		if (1) {
			itr->visited = visit ;
			edge * path = edge_alloc(itr->u, itr->v) ;
			for (node * g = itr->g; g; g = g->next)
				node_insert(&path->g, g->tid, g->m) ;
			node * next = itr->v ;
			edge * jtr = g->elist ;
			while (jtr) {
				if (jtr->u->m == next->m) {
					if (jtr->visited != visit) {
#ifdef DEBUG
	fprintf(stderr, "[PATH]\n") ;
	for (edge * p = path; p; p = p->next)
		printf("[(%ld, %p), (%ld, %p)]\n", p->u->tid, p->u->m, p->v->tid, p->v->m) ;
	printf("[(%ld, %p), (%ld, %p)]\n", jtr->u->tid, jtr->u->m, jtr->v->tid, jtr->v->m) ;
#endif
						edge * p = path ;
						while (p) {
							if (pthread_equal(p->u->tid, jtr->u->tid)) {
								break ;
							} else {
								p = p->next ;
							}
						} // while (p)
						if (p != 0x0) { // single thread
#ifdef DEBUG
	fprintf(stderr, "single thread\n") ;
#endif
							jtr = jtr->next ;
							continue ;
						}
						
						for (p = path; p; p = p->next) {
							node * pg = p->g ;
							for (pg = p->g; pg; pg = pg->next) {
								node * b = jtr->g ;
								for (b = jtr->g; b; b = b->next) {
									if (pg->m == b->m)
										break ;
								}
								if (b != 0x0)
									break ;
							}
							if (pg != 0x0)
								break ;
						} // for
						if (p != 0x0) { // guard lock
#ifdef DEBUG
	fprintf(stderr, "guard lock\n") ;
#endif
							jtr = jtr->next ;
							continue ;
						}

						jtr->visited = visit ;
						for (p = path; p->next; p = p->next) {
							if (p->u->m == jtr->v->m) {
								return 1 ;
							}
						}
						if (p->u->m == jtr->v->m) {
							return 1 ;
						}
						p->next = edge_alloc(jtr->u, jtr->v) ;
						for (node * g = jtr->g; g; g = g->next)
							node_insert(&p->next->g, g->tid, g->m) ;
						next = jtr->v ;
						jtr = g->elist ;
#if 1
	fprintf(stderr, "[PATH]\n") ;
	for (edge * p = path; p; p = p->next) {
		printf("[(%ld, %p), (%ld, %p)]\n", p->u->tid, p->u->m, p->v->tid, p->v->m) ;
#endif
	}
					} else {
						jtr = jtr->next ;	
					} // if (!jtr->visited)
				} else {
					jtr = jtr->next ;
				} // if (jtr->m == next->m)
			} // while (jtr)
			while (path) {
				edge_delete(&path, path->v->tid, path->v->m) ;
			}
		} // if (!itr->visited)
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
