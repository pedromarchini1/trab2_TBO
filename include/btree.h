#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>

typedef struct BTree BTree;

BTree *btree_create(int ordem, const char *filename);
BTree *btree_open(const char *filename);
void   btree_destroy(BTree *t);

void btree_insert(BTree *t, int chave, int registro);
int  btree_search(BTree *t, int chave, int *out_reg);
void btree_remove(BTree *t, int chave);
void btree_print(BTree *t, FILE *out);

#endif
