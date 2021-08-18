#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <errno.h>

#include "ddpredgraph.h"

node *
node_alloc (pthread_t tid, pthread_mutex_t * m)
{
//#ifdef DEBUG
//	fprintf(stderr, "node_alloc()\n") ;
//#endif
	node * n = (node *)malloc(sizeof(node)) ;
	if (n == 0x0)
		return 0x0 ;

	n->tid = tid ;
	n->m = m ;
	n->next = 0x0 ;
#ifdef DEBUG
	//fprintf(stderr, "[DEBUG] %p\n", n->m) ;
#endif
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

edge *
edge_alloc (node * u, node * v)
{
//#ifdef DEBUG
//	fprintf(stderr, "edge_alloc()\n") ;
//#endif
	edge * e = (edge *)malloc(sizeof(edge)) ;
	if (e == 0x0)
		return 0x0 ;
	e->u = node_alloc(u->tid, u->m) ;
	if (e->u == 0x0) {
		perror("edge_alloc") ;
		exit(EXIT_FAILURE) ;
	}
	e->v = node_alloc(v->tid, v->m) ;
	if (e->v == 0x0) {
		perror("edge_alloc") ;
		exit(EXIT_FAILURE) ;
	}
	e->g = 0x0 ;
	e->visited = 0 ;
	e->next = 0x0 ;

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
	while (e->g)
		node_delete(&e->g, e->g->tid, e->g->m) ;
	node_free(e->u) ;
	node_free(e->v) ;
	free(e) ;
}

int
node_search (node * nlist, pthread_t tid, pthread_mutex_t * m)
{
	node * itr = nlist ;

	while (itr) {
		if (pthread_equal(itr->tid, tid) && itr->m == m)
			return 1 ;
		itr = itr->next ;
	}
	return 0 ;
} /* nodelist_search */

int
node_insert (node ** nlist, pthread_t tid, pthread_mutex_t * m)
{
//	if (node_search(*nlist, tid, m))
//		return 0 ;

	node * tmp = node_alloc(tid, m) ;
	if (tmp == 0x0) {
		perror("node_alloc") ;
		exit(EXIT_FAILURE) ;
	}
	tmp->next = *nlist ;
	*nlist = tmp ;

#ifdef DEBUG
	//fprintf(stderr, "[DEBUG] nodelist insert %p\n", (*nlist)->n->m) ;
#endif
	return 1 ;
} /* nodelist_insert */

int
node_delete (node ** nlist, pthread_t tid, pthread_mutex_t * m)
{
	node * curr = *nlist ;
	node * prev = 0x0 ;
	if (curr != 0x0 && pthread_equal(curr->tid, tid) && curr->m == m) {
		*nlist = curr->next ;
		node_free(curr) ;
		return 1 ;
	}
	prev = curr ;
	while (curr) {
		if (pthread_equal(curr->tid, tid) && curr->m == m) {
			prev->next = curr->next ;
			node_free(curr) ;
			return 1 ;
		}
		prev = curr ;
		curr = curr->next ;
	}

	return 0 ;
} /* nodelist_delete */

int
edge_search (edge ** elist, node * u, node * v)
{
	edge * itr = *elist ;

	while (itr) {
		if (pthread_equal(itr->u->tid, u->tid) && itr->u->m == u->m && pthread_equal(itr->v->tid, v->tid) && itr->v->m == v->m)
			return 1 ;
		itr = itr->next ;
	}
	return 0 ;
} /* edgelist_search */

int
edge_insert (edge ** elist, node * u, node * v)
{
//#ifdef DEBUG
//	fprintf(stderr, "edge_insert()\n") ;
//#endif
	//if (edge_search(elist, u, v))
	//	return 0 ;

	edge * tmp = edge_alloc(u, v) ;
	if (tmp == 0x0) {
		perror("edge_alloc") ;
		exit(EXIT_FAILURE) ;
	}
	tmp->g = 0x0 ;
	tmp->next = *elist ;
	*elist = tmp ;
	return 1 ;
} /* edgelist_insert */

int
edge_delete (edge ** elist, pthread_t tid, pthread_mutex_t * m)
{
	edge * curr = *elist ;
	edge * prev = 0x0 ;
	while (curr != 0x0 && pthread_equal(curr->v->tid, tid) && curr->v->m == m) {
		*elist = curr->next ;
		edge_free(curr) ;
		curr = *elist ;
	}
	prev = curr ;
	while (curr) {
		if (pthread_equal(curr->v->tid, tid) && curr->v->m == m) {
			prev->next = curr->next ;
			edge_free(curr) ;
			curr = prev->next ;
		} else {
			prev = curr ;
			curr = curr->next ;
		}
	}

	return 0 ;
} /* edgelist_delete */

graph *
graph_init ()
{
	graph * g = (graph *)malloc(sizeof(graph)) ;
	if (g == 0x0)
		return g ;
	
	g->nlist = 0x0 ;
	g->elist = 0x0 ;

	return g ;
} /* graph_init */

void
graph_print (graph * g)
{
	node * n_itr = g->nlist ;
	edge * e_itr = g->elist ;

	printf("[NODES]\n") ;
	while (n_itr) {
		printf("(%ld, %p)\n", n_itr->tid, n_itr->m) ;
		n_itr = n_itr->next ;
	}
	printf("[EDGES]\n") ;
	while (e_itr) {
		printf("[(%ld, %p), (%ld, %p)]\n", e_itr->u->tid, e_itr->u->m, e_itr->v->tid, e_itr->v->m) ;
#ifdef DEBUG
	for (node * g = e_itr->g; g; g = g->next)
		printf("(%p)-", g->m) ;
	printf("\n") ;
#endif
		e_itr = e_itr->next ;
	}
} /* graph_print */

int
graph_lock (graph * g, pthread_t tid, pthread_mutex_t * m)
{
	if (!node_insert(&g->nlist, tid, m)) {
		printf("RELOCK!\n") ;
		return 0 ;
	}
	node * itr = g->nlist->next ; // new node is g->nlist so no need to create edge
	while (itr) {
//#ifdef DEBUG
//	fprintf(stderr, "graph_lock: while (itr)\n") ;
//#endif
		if (pthread_equal(itr->tid, tid)) {
			edge_insert(&g->elist, itr, g->nlist) ;
			for (node * jtr = g->nlist->next; jtr; jtr = jtr->next) {
				if (pthread_equal(jtr->tid, tid)) {
					node_insert(&g->elist->g, jtr->tid, jtr->m) ;
				}
			}
		}
		itr = itr->next ;
	}
	return 1 ;
} /* graph_lock */

void
graph_unlock (graph * g, pthread_t tid, pthread_mutex_t * m)
{
	//edge_delete(&g->elist, tid, m) ;
#ifdef DEBUG	
	//fprintf(stderr, "[DEBUG] edgelist_delete\n") ;
	//graph_print(g) ;
#endif
	node_delete(&g->nlist, tid, m) ;
#ifdef DEBUG
	//fprintf(stderr, "[DEBUG] nodelist_delete\n") ;
	//graph_print(g) ;
#endif
} /* graph_unlock */

int
graph_detect (graph * g)
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
			} // while(curr)
		}
		itr = itr->next ;
		++visit ;
	} // while(itr) 
	return 0 ;
} /* graph_detect */

void
detected (graph * g, char * fname, long int addr)
{
	graph_print(g) ;
	printf("DEADLOCK!\n") ;
		
	char command[1024] ;
	char buf[1024] ;
	snprintf(command, 1024, "addr2line -f -e %s %lx", fname, addr) ;
	FILE * fp = popen(command, "r") ;
	while (fgets(buf, 1024, fp) != 0x0)
		printf("%s", buf) ;
	pclose(fp) ;
} /* detected */
