#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <errno.h>

#include "ddgraph.h"

node *
node_alloc (pthread_t tid, pthread_mutex_t * m)
{
	node * n = (node *)malloc(sizeof(node)) ;
	if (n == 0x0)
		return 0x0 ;

	n->tid = tid ;
	n->m = m ;
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
	edge * e = (edge *)malloc(sizeof(edge)) ;
	if (e == 0x0)
		return 0x0 ;
	e->u = u ;
	e->v = v ;
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
	e->u = 0x0 ;
	e->v = 0x0 ;
	free(e) ;
}

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
} /* nodelist_search */

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

#ifdef DEBUG
	//fprintf(stderr, "[DEBUG] nodelist insert %p\n", (*nlist)->n->m) ;
#endif
	return 1 ;
} /* nodelist_insert */

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
} /* nodelist_delete */

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
} /* edgelist_search */

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
	tmp->e = edge_alloc(u, v) ;
	if (tmp->e == 0x0) {
		perror("edge_alloc") ;
		exit(EXIT_FAILURE) ;
	}
	tmp->next = *elist ;
	*elist = tmp ;
	return 1 ;
} /* edgelist_insert */

int
edgelist_delete (edgelist ** elist, pthread_t tid, pthread_mutex_t * m)
{
	edgelist * curr = *elist ;
	edgelist * prev = 0x0 ;
	while (curr != 0x0 && pthread_equal(curr->e->v->tid, tid) && curr->e->v->m == m) {
		*elist = curr->next ;
		edge_free(curr->e) ;
		free(curr) ;
		curr = *elist ;
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
} /* graph_print */

int
graph_lock (graph * g, pthread_t tid, pthread_mutex_t * m)
{
	if (!nodelist_insert(&g->nlist, tid, m)) {
		printf("RELOCK!\n") ;
		return 0 ;
	}
#ifdef DEBUG
	//fprintf(stderr, "[DEBUG] lock_dep LOCK after nodelistinsert\n") ;
	//fprintf(stderr, "[DEBUG] %p\n", g->nlist->n->m) ;
#endif
	nodelist * itr = g->nlist->next ; // new node is g->nlist so no need to create edge
	while (itr) {
		if (pthread_equal(itr->n->tid, tid)) {
			edgelist_insert(&g->elist, itr->n, g->nlist->n) ;
		}
		itr = itr->next ;
	}
	return 1 ;
} /* graph_lock */

void
graph_unlock (graph * g, pthread_t tid, pthread_mutex_t * m)
{
	edgelist_delete(&g->elist, tid, m) ;
#ifdef DEBUG	
	//fprintf(stderr, "[DEBUG] edgelist_delete\n") ;
	//graph_print(g) ;
#endif
	nodelist_delete(&g->nlist, tid, m) ;
#ifdef DEBUG
	//fprintf(stderr, "[DEBUG] nodelist_delete\n") ;
	//graph_print(g) ;
#endif
} /* graph_unlock */

int
graph_detect (graph * g)
{
	edgelist * itr = g->elist ;
	while (itr) {
		itr->e->visited = 0 ;
		itr = itr->next ;
	}
	itr = g->elist ;
	int visit = 1 ;
	while (itr) {
		if (!itr->e->visited) {
			itr->e->visited = visit ;
			node * next = itr->e->v ;
			edgelist * curr = g->elist ;
			while (curr) {
				if (curr->e->u->m == next->m) {
					if (curr->e->visited == visit) {
						return 1 ;
					} else {
						curr->e->visited = visit ;
						next = curr->e->v ;
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
