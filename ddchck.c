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

typedef struct node_t {
	pthread_t tid ;
	pthread_mutex_t * m ;
} node ;

node *
node_alloc (pthread_t tid, pthread_mutex_t * m)
{
	node * n = (node *)malloc(sizeof(node)) ;
	if (n == 0x0)
		return 0x0 ;

	n->tid = tid ;
	n->m = m ;
//fprintf(stderr, "[DEBUG] %p\n", n->m) ;
	return n ;
}

int
node_equal (node * n, node * m)
{
	return (n->tid == m->tid && n->m == m->m) ;
}

void
node_free (node * n)
{
	free(n) ;
}

typedef struct edge_t {
	node * u ;
	node * v ;
	int visited ;
} edge ;

edge *
edge_alloc (node ** u, node ** v)
{
	edge * e = (edge *)malloc(sizeof(edge)) ;
	if (e == 0x0)
		return 0x0 ;
	e->u = *u ;
	e->v = *v ;
	e->visited = 0 ;

	return e ;
}

int
edge_equal (edge * e, edge * f)
{
	return (node_equal(e->u, f->u) && node_equal(e->v, f->v)) ;
}

void
edge_free (edge * e)
{
	free(e) ;
}

typedef struct nodelist_t {
	node * n ;
	struct nodelist_t * next ;
	int size ;
} nodelist ;

int
nodelist_search (nodelist ** nlist, pthread_t tid, pthread_mutex_t * m)
{
	nodelist * itr = *nlist ;

	while (itr) {
		if (pthread_equal(itr->n->tid, tid) && itr->n->m == m)
			return 1 ;
		itr = itr->next ;
	}
	return 0 ;
}

int
nodelist_insert (nodelist ** nlist, pthread_t tid, pthread_mutex_t * m)
{
	if (nodelist_search(nlist, tid, m))
		return 0 ;

	nodelist * tmp = (nodelist *)malloc(sizeof(nodelist)) ;
	if (tmp == 0x0) {
		perror("nodelist_insert") ;
		exit(EXIT_FAILURE) ;
	}
	tmp->n = node_alloc(tid, m) ;
	if (tmp->n == 0x0) {
		perror("node_alloc") ;
		exit(EXIT_FAILURE) ;
	}
	tmp->next = *nlist ;
	*nlist = tmp ;

//	fprintf(stderr, "[DEBUG] nodelist insert %p\n", (*nlist)->n->m) ;
	return 1 ;
}

int
nodelist_delete (nodelist ** nlist, pthread_t tid, pthread_mutex_t * m)
{
	nodelist * curr = *nlist ;
	nodelist * prev = 0x0 ;
	if (curr != 0x0 && pthread_equal(curr->n->tid, tid) && curr->n->m == m) {
		*nlist = curr->next ;
		node_free(curr->n) ;
		free(curr) ;
		return 1 ;
	}
	prev = curr ;
	while (curr) {
		if (pthread_equal(curr->n->tid, tid) && curr->n->m == m) {
			prev->next = curr->next ;
			node_free(curr->n) ;
			free(curr) ;
			return 1 ;
		}
		prev = curr ;
		curr = curr->next ;
	}

	return 0 ;
}

typedef struct edgelist_t {
	edge * e ;
	struct edgelist_t * next ;
	int size ;
} edgelist ;

int
edgelist_search (edgelist ** elist, node * u, node * v)
{
	edgelist * itr = *elist ;

	while (itr) {
		if (pthread_equal(itr->e->u->tid, u->tid) && itr->e->u->m == u->m && pthread_equal(itr->e->v->tid, v->tid) && itr->e->v->m == v->m)
			return 1 ;
		itr = itr->next ;
	}
	return 0 ;
}

int
edgelist_insert (edgelist ** elist, node * u, node * v)
{
	if (edgelist_search(elist, u, v))
		return 0 ;
	
	edgelist * tmp = (edgelist *)malloc(sizeof(edgelist)) ;
	if (tmp == 0x0) {
		perror("edgelist_insert") ;
		exit(EXIT_FAILURE) ;
	}
	tmp->e = edge_alloc(&u, &v) ;
	if (tmp->e == 0x0) {
		perror("edge_alloc") ;
		exit(EXIT_FAILURE) ;
	}
	tmp->next = *elist ;
	*elist = tmp ;
	return 1 ;
}

int
edgelist_delete (edgelist ** elist, pthread_t tid, pthread_mutex_t * m)
{
	edgelist * curr = *elist ;
	edgelist * prev = 0x0 ;
	if (curr != 0x0 && pthread_equal(curr->e->v->tid, tid) && curr->e->v->m == m) {
		*elist = curr->next ;
		edge_free(curr->e) ;
		free(curr) ;
		return 1 ;
	}
	prev = curr ;
	while (curr) {
		if (pthread_equal(curr->e->v->tid, tid) && curr->e->v->m == m) {
			prev->next = curr->next ;
			edge_free(curr->e) ;
			free(curr) ;
			curr = prev->next ;
		} else {
			prev = curr ;
			curr = curr->next ;
		}
	}

	return 0 ;
}

typedef struct _graph {
	nodelist * nlist ;
	edgelist * elist ;
} graph ;

graph *
graph_init ()
{
	graph * g = (graph *)malloc(sizeof(graph)) ;
	if (g == 0x0)
		return g ;
	
	g->nlist = 0x0 ;
	g->elist = 0x0 ;

	return g ;
}

void
graph_print (graph * g)
{
	nodelist * n_itr = g->nlist ;
	edgelist * e_itr = g->elist ;

	printf("[NODES]\n") ;
	while (n_itr) {
		printf("(%ld, %p)\n", n_itr->n->tid, n_itr->n->m) ;
		n_itr = n_itr->next ;
	}
	printf("[EDGES]\n") ;
	while (e_itr) {
		printf("[(%ld, %p), (%ld, %p)]\n", e_itr->e->u->tid, e_itr->e->u->m, e_itr->e->v->tid, e_itr->e->v->m) ;
		e_itr = e_itr->next ;
	}
}

void
lock_dep (graph * g, int mode, pthread_t tid, pthread_mutex_t * m)
{
	if (mode == LOCK) {

		nodelist_insert(&g->nlist, tid, m) ;

//		fprintf(stderr, "[DEBUG] lock_dep LOCK after nodelistinsert\n") ;
//fprintf(stderr, "[DEBUG] %p\n", g->nlist->n->m) ;


		nodelist * itr = g->nlist->next ; // new node is g->nlist so no need to create edge
		while (itr) {
			if (pthread_equal(itr->n->tid, tid)) {
				edgelist_insert(&g->elist, itr->n, g->nlist->n) ;
			}
			itr = itr->next ;
		}
//		fprintf(stderr, "[DEBUG] lock_dep LOCK end\n") ;

	} else if (mode == UNLOCK) {
		edgelist_delete(&g->elist, tid, m) ;
		fprintf(stderr, "[DEBUG] edgelist_delete\n") ;
		graph_print(g) ;
		
		nodelist_delete(&g->nlist, tid, m) ;
		fprintf(stderr, "[DEBUG] nodelist_delete\n") ;
//		fprintf(stderr, "[DEBUG] lock_dep UNLOCK\n") ;
	} else {
		perror("unkown mode") ;
		exit(EXIT_FAILURE) ;
	}
/**
	edgelist * itr = glist->elist ;
	while (itr) {
		itr->e->visited = 0 ;
		itr = itr->next ;
	}
	itr = glist->elist ;
	while (itr) {
		if (!itr->e->visited) {
			itr->e->visited = 1 ;
			node * start = itr->e->u ;
			node * next = itr->e->v ;
			edgelist * curr = glist->elist ;
			while (curr) {
				if (node_equal(curr->e->u, next)) {
					
				}
			}

		}
	}
**/
}

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

	graph * lockgraph = graph_init() ;
	if (lockgraph == 0x0) {
		perror("graph_init()") ;
		exit(EXIT_FAILURE) ;
	}

	while (1) {
		int mode ;
		pthread_t tid ;
		pthread_mutex_t * mutex ;
		flock(fd, LOCK_EX) ;
			int ret = ddread(fd, &mode, &tid, &mutex) ;
		flock(fd, LOCK_UN) ;
	
		if (ret) {
			printf("[READ] %d %ld %p\n", mode, tid, mutex) ;
			lock_dep(lockgraph, mode, tid, mutex) ;
			graph_print(lockgraph) ;
		}
	}	
	close(fd) ;
}