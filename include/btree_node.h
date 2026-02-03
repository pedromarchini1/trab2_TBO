#ifndef BTREE_NODE_H
#define BTREE_NODE_H

#include <stdio.h>

typedef struct BTreeNode {
    int is_leaf;
    int n_chaves;

    int *chaves;
    int *registros;
    long *filhos;

    int ordem;
    int *removido;
} BTreeNode;

/* criação / destruição */
BTreeNode *node_create(int ordem, int is_leaf);
void node_destroy(BTreeNode *n);

/* utilidades */
int node_is_full(BTreeNode *n, int ordem);

/* arquivo */
void node_write(FILE *fp, long offset, BTreeNode *n);
void node_read(FILE *fp, long offset, BTreeNode *n);

/* debug */
void node_print(BTreeNode *n);

long node_disk_size(int ordem);


#endif
