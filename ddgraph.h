#ifndef DDGRAPH_H
#define DDGRAPH_H

typedef struct node_t {
	pthread_t tid ;
	pthread_mutex_t * m ;
	struct node_t * next ;
} node ;

typedef struct edge_t {
	node * u ;
	node * v ;
	int visited ;
	struct edge_t * next ;
} edge ;

typedef struct _graph {
	node * nlist ;
	edge * elist ;
} graph ;

node *
node_alloc (pthread_t tid, pthread_mutex_t * m) ;

int
node_equal (node * n, node * m) ;

void
node_free (node * n) ;

edge *
edge_alloc (node * u, node * v) ;

int
edge_equal (edge * e, edge * f) ;

void
edge_free (edge * e) ;

int
node_search (node ** nlist, pthread_t tid, pthread_mutex_t * m) ;

int
node_insert (node ** nlist, pthread_t tid, pthread_mutex_t * m) ;

int
node_delete (node ** nlist, pthread_t tid, pthread_mutex_t * m) ;

int
edge_search (edge ** elist, node * u, node * v) ;

int
edge_insert (edge ** elist, node * u, node * v) ;

int
edge_delete (edge ** elist, pthread_t tid, pthread_mutex_t * m) ;

graph *
graph_init () ;

void
graph_print (graph * g) ;

int
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
