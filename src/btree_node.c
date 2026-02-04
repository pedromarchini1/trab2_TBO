#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree_node.h"

BTreeNode *node_create(int ordem, int is_leaf) {
    BTreeNode *n = malloc(sizeof(BTreeNode));
    if (!n) return NULL;

    n->ordem = ordem;
    n->is_leaf = is_leaf;
    n->n_chaves = 0;

    n->chaves = calloc(ordem - 1, sizeof(int));
    n->registros = calloc(ordem - 1, sizeof(int));
    n->filhos = calloc(ordem, sizeof(long));
    n->removido = calloc(ordem, sizeof(int));

    for (int i = 0; i < ordem; i++)
        n->filhos[i] = -1;

    for (int i = 0; i < ordem - 1; i++)
        n->removido[i] = 0;
    return n;
}


void node_destroy(BTreeNode *n) {
    if (!n) return;
    free(n->chaves);
    free(n->registros);
    free(n->filhos);
    free(n->removido);
    free(n);
}


int node_is_full(BTreeNode *n, int ordem) {
    return n->n_chaves == ordem - 1;
}


void node_write(FILE *fp, long offset, BTreeNode *n)
{
    fseek(fp, offset, SEEK_SET);

    fwrite(&n->is_leaf, sizeof(int), 1, fp);
    fwrite(&n->n_chaves, sizeof(int), 1, fp);

    fwrite(n->chaves, sizeof(int), n->ordem - 1, fp);
    fwrite(n->registros, sizeof(int), n->ordem - 1, fp);
    fwrite(n->removido, sizeof(int), n->ordem - 1, fp);
    fwrite(n->filhos, sizeof(long), n->ordem, fp);

    fflush(fp);
}



void node_read(FILE *fp, long offset, BTreeNode *n)
{
    fseek(fp, offset, SEEK_SET);

    fread(&n->is_leaf, sizeof(int), 1, fp);
    fread(&n->n_chaves, sizeof(int), 1, fp);

    fread(n->chaves, sizeof(int), n->ordem - 1, fp);
    fread(n->registros, sizeof(int), n->ordem - 1, fp);
    fread(n->removido, sizeof(int), n->ordem - 1, fp);
    fread(n->filhos, sizeof(long), n->ordem, fp);
}



void node_print(BTreeNode *n) {
    printf("[");
    for (int i = 0; i < n->n_chaves; i++) {
        printf("%d", n->chaves[i]);
        if (i + 1 < n->n_chaves)
            printf(" | ");
    }
    printf("]");
}

long node_disk_size(int ordem)
{
    return
        sizeof(int) +                 // is_leaf
        sizeof(int) +                 // n_chaves
        (ordem - 1) * sizeof(int) +   // chaves
        (ordem - 1) * sizeof(int) +   // registros
        (ordem - 1) * sizeof(int) +   // removido  
        ordem * sizeof(long);         // filhos
}

