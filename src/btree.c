#include <stdio.h>
#include <stdlib.h>
#include "btree.h"
#include "btree_node.h"

struct BTree
{
    int ordem;
    long raiz;
    long prox_no;
    FILE *fp;
};

BTree *btree_create(int ordem, const char *filename)
{
    BTree *t = malloc(sizeof(BTree));
    t->ordem = ordem;
    t->prox_no = 0;
    t->fp = fopen(filename, "w+b");

    BTreeNode *root = node_create(ordem, 1);
    t->raiz = t->prox_no++;

    node_write(t->fp,
               t->raiz * node_disk_size(t->ordem),
               root);

    node_destroy(root);

    return t;
}

void btree_destroy(BTree *t)
{
    if (!t)
        return;
    fclose(t->fp);
    free(t);
}

void btree_split_child(BTree *t,
                       BTreeNode *pai,
                       long idx_pai,
                       int i,
                       long idx_filho)
{
    int m = t->ordem;
    int mid = (m - 1) / 2;
    long off = node_disk_size(m);

    /* lê o filho cheio */
    BTreeNode *y = node_create(m, 0);
    node_read(t->fp, idx_filho * off, y);

    /* cria novo nó z */
    BTreeNode *z = node_create(m, y->is_leaf);
    z->n_chaves = (m - 1) - mid - 1;

    /* copia chaves/registros */
    for (int j = 0; j < z->n_chaves; j++)
    {
        z->chaves[j] = y->chaves[mid + 1 + j];
        z->registros[j] = y->registros[mid + 1 + j];
        z->removido[j] = y->removido[mid + 1 + j];
    }

    /* copia filhos */
    if (!y->is_leaf)
    {
        for (int j = 0; j <= z->n_chaves; j++)
            z->filhos[j] = y->filhos[mid + 1 + j];
    }

    y->n_chaves = mid;

    /* abre espaço no pai */
    for (int j = pai->n_chaves; j >= i + 1; j--)
        pai->filhos[j + 1] = pai->filhos[j];

    long idx_z = t->prox_no++;
    pai->filhos[i + 1] = idx_z;

    for (int j = pai->n_chaves - 1; j >= i; j--)
    {
        pai->chaves[j + 1] = pai->chaves[j];
        pai->registros[j + 1] = pai->registros[j];
        pai->removido[j + 1] = pai->removido[j];
    }

    /* promove a chave do meio */
    pai->chaves[i] = y->chaves[mid];
    pai->registros[i] = y->registros[mid];
    pai->removido[i] = y->removido[mid];
    pai->n_chaves++;

    /* escreve tudo */
    node_write(t->fp, idx_filho * off, y);
    node_write(t->fp, idx_z * off, z);
    node_write(t->fp, idx_pai * off, pai);

    node_destroy(y);
    node_destroy(z);
}

static void insert_non_full(BTree *t,
                            BTreeNode *x,
                            long idx_x,
                            int k,
                            int reg)
{
    long off = node_disk_size(t->ordem);
    int i = x->n_chaves - 1;

    if (x->is_leaf)
    {
        while (i >= 0 && k < x->chaves[i])
        {
            x->chaves[i + 1] = x->chaves[i];
            x->registros[i + 1] = x->registros[i];
            x->removido[i + 1] = x->removido[i];
            i--;
        }

        x->chaves[i + 1] = k;
        x->registros[i + 1] = reg;
        x->removido[i + 1] = 0;
        x->n_chaves++;

        node_write(t->fp, idx_x * off, x);
    }
    else
    {
        while (i >= 0 && k < x->chaves[i])
            i--;
        i++;

        long idx_filho = x->filhos[i];
        BTreeNode *child = node_create(t->ordem, 0);
        node_read(t->fp, idx_filho * off, child);

        if (node_is_full(child, t->ordem))
        {
            btree_split_child(t, x, idx_x, i, idx_filho);

            /* recarrega o pai após split */
            node_read(t->fp, idx_x * off, x);

            if (k > x->chaves[i])
                i++;

            idx_filho = x->filhos[i];
            node_read(t->fp, idx_filho * off, child);
        }

        insert_non_full(t, child, idx_filho, k, reg);
        node_destroy(child);
    }
}

void btree_insert(BTree *t, int chave, int registro)
{
    long off = node_disk_size(t->ordem);

    /* lê a raiz */
    BTreeNode *root = node_create(t->ordem, 0);
    node_read(t->fp, t->raiz * off, root);

    /* se raiz está cheia, cria nova raiz */
    if (node_is_full(root, t->ordem))
    {
        BTreeNode *new_root = node_create(t->ordem, 0);
        new_root->is_leaf = 0;
        new_root->filhos[0] = t->raiz;

        long idx_new_root = t->prox_no++;

        btree_split_child(t,
                          new_root,
                          idx_new_root,
                          0,
                          t->raiz);

        t->raiz = idx_new_root;

        insert_non_full(t, new_root, idx_new_root, chave, registro);

        node_write(t->fp, idx_new_root * off, new_root);
        node_destroy(new_root);
    }
    else
    {
        insert_non_full(t, root, t->raiz, chave, registro);
    }

    node_destroy(root);
}



static int search_rec(BTree *t, long idx, int k, int *out_reg)
{
    BTreeNode *x = node_create(t->ordem, 0);
    node_read(t->fp,
              idx * node_disk_size(t->ordem),
              x);

    int i = 0;
    while (i < x->n_chaves && k > x->chaves[i])
        i++;

    if (i < x->n_chaves && k == x->chaves[i])
    {
        if (!x->removido[i])
        {
            *out_reg = x->registros[i];
            return 1;
        }
        return 0; // chave existe mas foi removida
    }

    if (x->is_leaf)
    {
        node_destroy(x);
        return 0;
    }

    long next = x->filhos[i];
    node_destroy(x);
    return search_rec(t, next, k, out_reg);
}

int btree_search(BTree *t, int chave, int *out_reg)
{
    return search_rec(t, t->raiz, chave, out_reg);
}

void btree_print(BTree *t, FILE *out)
{
    if (!t || t->raiz < 0)
        return;

    long queue[1024];
    int ini = 0, fim = 0;

    queue[fim++] = t->raiz;

    while (ini < fim)
    {
        int level_size = fim - ini;

        while (level_size--)
        {
            long idx = queue[ini++];

            BTreeNode *n = node_create(t->ordem, 0);
            node_read(
                t->fp,
                idx * node_disk_size(t->ordem),
                n);

            /* imprime o nó */
            fprintf(out, "[");
            for (int i = 0; i < n->n_chaves; i++)
            {
                if (!n->removido[i])
                    fprintf(out, "key: %d, ", n->chaves[i]);
            }

            fprintf(out, "]");

            /* enfileira filhos */
            if (!n->is_leaf)
            {
                for (int i = 0; i <= n->n_chaves; i++)
                    queue[fim++] = n->filhos[i];
            }

            node_destroy(n);
        }
        fprintf(out, "\n");
    }
}

static void remove_rec(BTree *t, long idx, int chave)
{
    long off = node_disk_size(t->ordem);

    BTreeNode *n = node_create(t->ordem, 0);
    node_read(t->fp, idx * off, n);

    int i = 0;
    while (i < n->n_chaves && chave > n->chaves[i])
        i++;

    if (i < n->n_chaves && chave == n->chaves[i])
    {
        n->removido[i] = 1;
        node_write(t->fp, idx * off, n);
        node_destroy(n);
        return;
    }

    if (n->is_leaf)
    {
        node_destroy(n);
        return;
    }

    long next = n->filhos[i];
    node_destroy(n);
    remove_rec(t, next, chave);
}

void btree_remove(BTree *t, int chave)
{
    remove_rec(t, t->raiz, chave);
}

