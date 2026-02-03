#include <stdio.h>
#include <stdlib.h>
#include "btree.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <entrada.txt> <saida.txt>\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");

    if (!in || !out) {
        fprintf(stderr, "Erro ao abrir arquivos.\n");
        return 1;
    }

    int ordem, n;
    fscanf(in, "%d", &ordem);
    fscanf(in, "%d", &n);

    BTree *t = btree_create(ordem, "btree.bin");
    if (!t) {
        fprintf(stderr, "Erro ao criar arvore B.\n");
        fclose(in);
        fclose(out);
        return 1;
    }

    char op;
    int chave, reg;

    for (int i = 0; i < n; i++) {
        fscanf(in, " %c", &op);

        if (op == 'I') {
            fscanf(in, "%d%*[, ]%d", &chave, &reg);
            btree_insert(t, chave, reg);
        }
        else if (op == 'R') {
            fscanf(in, "%d", &chave);
            btree_remove(t, chave);
        }
        else if (op == 'B') {
            fscanf(in, "%d", &chave);
            if (btree_search(t, chave, &reg))
                fprintf(out, "O REGISTRO ESTA NA ARVORE!\n");
            else
                fprintf(out, "O REGISTRO NAO ESTA NA ARVORE!\n");
        }
    }

    fprintf(out, "-- ARVORE B\n");
    btree_print(t, out);

    btree_destroy(t);
    fclose(in);
    fclose(out);

    return 0;
}
