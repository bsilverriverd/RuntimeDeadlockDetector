#ifndef DDGRAPH_H
#define DDGRAPH_H

typedef struct node_t {
	pthread_t tid ;
	pthread_mutex_t * m ;
} node ;

typedef struct edge_t {
	node * u ;
	node * v ;
	int visited ;
} edge ;

typedef struct nodelist_t {
	node * n ;
	struct nodelist_t * next ;
	int size ;
} nodelist ;

typedef struct edgelist_t {
	edge * e ;
	struct edgelist_t * next ;
	int size ;
} edgelist ;

typedef struct _graph {
	nodelist * nlist ;
	edgelist * elist ;
} graph ;

node *
node_alloc (pthread_t tid, pthread_mutex_t * m) ;

int
node_equal (node * n, node * m) ;

void
node_free (node * n) ;

edge *
edge_alloc (node ** u, node ** v) ;

int
edge_equal (edge * e, edge * f) ;

void
edge_free (edge * e) ;

int
nodelist_search (nodelist ** nlist, pthread_t tid, pthread_mutex_t * m) ;

int
nodelist_insert (nodelist ** nlist, pthread_t tid, pthread_mutex_t * m) ;

int
nodelist_delete (nodelist ** nlist, pthread_t tid, pthread_mutex_t * m) ;

int
edgelist_search (edgelist ** elist, node * u, node * v) ;

int
edgelist_insert (edgelist ** elist, node * u, node * v) ;

int
edgelist_delete (edgelist ** elist, pthread_t tid, pthread_mutex_t * m) ;

graph *
graph_init () ;

void
graph_print (graph * g) ;

void
graph_lock (graph * g, pthread_t tid, pthread_mutex_t * m) ;

void
graph_unlock (graph * g, pthread_t tid, pthread_mutex_t * m) ;

int
graph_detect (graph * g) ;

int
lock_dep (graph * g, int mode, pthread_t tid, pthread_mutex_t * m) ;

void
detected (graph * g, char * fname, long int addr) ;
#endif
