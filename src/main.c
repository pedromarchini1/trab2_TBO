#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr,
                "Uso: %s <entrada.txt> <saida.txt>\n",
                argv[0]);
        return 1;
    }

    /* =====================================================
       ABERTURA DOS ARQUIVOS
       ===================================================== */

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        fprintf(stderr, "Erro ao abrir arquivo de entrada.\n");
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        fprintf(stderr, "Erro ao abrir arquivo de saída.\n");
        fclose(in);
        return 1;
    }

    /* =====================================================
       LEITURA DOS PARÂMETROS
       ===================================================== */

    int ordem, n;

    if (fscanf(in, "%d", &ordem) != 1 ||
        fscanf(in, "%d", &n) != 1) {
        fprintf(stderr, "Erro ao ler cabeçalho.\n");
        fclose(in);
        fclose(out);
        return 1;
    }

    /* =====================================================
       CRIAÇÃO DA ÁRVORE
       ===================================================== */

    BTree *t = btree_create(ordem, "btree.bin");

    if (!t) {
        fprintf(stderr, "Erro ao criar árvore.\n");
        fclose(in);
        fclose(out);
        return 1;
    }

    /* =====================================================
       PROCESSAMENTO DAS OPERAÇÕES
       ===================================================== */

    char op;
    int chave, reg;

    for (int i = 0; i < n; i++) {

        if (fscanf(in, " %c", &op) != 1)
            break;

        switch (op) {

            case 'I':   /* Inserção */
                fscanf(in, "%d%*[, ]%d", &chave, &reg);
                btree_insert(t, chave, reg);
                break;

            case 'R':   /* Remoção */
                fscanf(in, "%d", &chave);
                btree_remove(t, chave);
                break;

            case 'B':   /* Busca */
                fscanf(in, "%d", &chave);

                if (btree_search(t, chave, &reg))
                    fprintf(out,
                            "O REGISTRO ESTA NA ARVORE!\n");
                else
                    fprintf(out,
                            "O REGISTRO NAO ESTA NA ARVORE!\n");
                break;

            default:
                fprintf(stderr,
                        "Operação desconhecida: %c\n", op);
        }
    }

    /* =====================================================
       IMPRESSÃO FINAL
       ===================================================== */

    fprintf(out, "-- ARVORE B\n");
    btree_print(t, out);

    /* =====================================================
       FINALIZAÇÃO
       ===================================================== */

    btree_destroy(t);
    fclose(in);
    fclose(out);

    return 0;
}
