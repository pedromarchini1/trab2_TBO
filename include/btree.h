#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>

/* Estrutura opaca da Árvore B */
typedef struct BTree BTree;

/* ===============================
   Criação / destruição
   =============================== */

/* Cria uma árvore B de dada ordem associada a um arquivo binário */
BTree *btree_create(int ordem, const char *filename);

/* Fecha o arquivo e libera a árvore */
void btree_destroy(BTree *t);

/* ===============================
   Operações
   =============================== */

/* Insere uma chave e um registro associado */
void btree_insert(BTree *t, int chave, int registro);

/* Busca uma chave (retorna 1 se achou, 0 caso contrário)
   Se achou, escreve o registro em *out_reg */
int btree_search(BTree *t, int chave, int *out_reg);

/* Impressão em largura (BFS) – debug */
void btree_print(BTree *t, FILE *out);

void btree_remove(BTree *t, int chave);


#endif
