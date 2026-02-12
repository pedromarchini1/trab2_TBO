#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"
#include "btree_node.h"
#include "queue.h"

typedef struct
{
    int ordem;
    long raiz;
    long prox_no;
} BTreeHeader;

struct BTree
{
    BTreeHeader header;
    FILE *fp;
};

static long node_offset(BTree *t, long idx)
{
    return sizeof(BTreeHeader) +
           idx * node_disk_size(t->header.ordem);
}

static void write_header(BTree *t)
{
    fseek(t->fp, 0, SEEK_SET);
    fwrite(&t->header, sizeof(BTreeHeader), 1, t->fp);
    fflush(t->fp);
}

static void read_header(BTree *t)
{
    fseek(t->fp, 0, SEEK_SET);
    fread(&t->header, sizeof(BTreeHeader), 1, t->fp);
}

static BTreeNode *read_node(BTree *t, long idx)
{
    BTreeNode *n = node_create(t->header.ordem, 0);
    node_read(t->fp, node_offset(t, idx), n);
    return n;
}

static void write_node(BTree *t, long idx, BTreeNode *n)
{
    node_write(t->fp, node_offset(t, idx), n);
}

BTree *btree_create(int ordem, const char *filename)
{
    BTree *t = malloc(sizeof(BTree));
    if (!t)
        return NULL;

    t->fp = fopen(filename, "w+b");
    if (!t->fp)
    {
        free(t);
        return NULL;
    }

    t->header.ordem = ordem;
    t->header.raiz = 0;
    t->header.prox_no = 1;

    write_header(t);

    BTreeNode *root = node_create(ordem, 1);
    write_node(t, 0, root);
    node_destroy(root);

    return t;
}

BTree *btree_open(const char *filename)
{
    BTree *t = malloc(sizeof(BTree));
    if (!t)
        return NULL;

    t->fp = fopen(filename, "r+b");
    if (!t->fp)
    {
        free(t);
        return NULL;
    }

    read_header(t);
    return t;
}

void btree_destroy(BTree *t)
{
    if (!t)
        return;
    write_header(t);
    fclose(t->fp);
    free(t);
}


static int search_rec(BTree *t, long idx, int key, int *out_reg)
{
    BTreeNode *n = read_node(t, idx);
    int i = 0;

    while (i < n->n_chaves && key > n->chaves[i])
        i++;

    if (i < n->n_chaves && key == n->chaves[i])
    {
        *out_reg = n->registros[i];
        node_destroy(n);
        return 1;
    }

    if (n->is_leaf)
    {
        node_destroy(n);
        return 0;
    }

    long next = n->filhos[i];
    node_destroy(n);
    return search_rec(t, next, key, out_reg);
}

int btree_search(BTree *t, int chave, int *out_reg)
{
    return search_rec(t, t->header.raiz, chave, out_reg);
}


static void split_child(BTree *t, long parent_idx,
                        BTreeNode *parent, int i)
{
    int ordem = t->header.ordem;
    int mid = (t->header.ordem - 1) / 2;

    long y_idx = parent->filhos[i];
    BTreeNode *y = read_node(t, y_idx);

    long z_idx = t->header.prox_no++;
    BTreeNode *z = node_create(ordem, y->is_leaf);

    z->n_chaves =
        y->n_chaves - mid - 1;

    for (int j = 0;
         j < z->n_chaves;
         j++)
    {
        z->chaves[j] =
            y->chaves[j + mid + 1];

        z->registros[j] =
            y->registros[j + mid + 1];
    }

    if (!y->is_leaf)
    {
        for (int j = 0;
             j <= z->n_chaves;
             j++)
        {
            z->filhos[j] =
                y->filhos[j + mid + 1];
        }
    }

    y->n_chaves = mid;

    for (int j = parent->n_chaves; j >= i + 1; j--)
        parent->filhos[j + 1] = parent->filhos[j];

    parent->filhos[i + 1] = z_idx;

    for (int j = parent->n_chaves - 1; j >= i; j--)
    {
        parent->chaves[j + 1] = parent->chaves[j];
        parent->registros[j + 1] = parent->registros[j];
    }

    parent->chaves[i] = y->chaves[mid];
    parent->registros[i] = y->registros[mid];
    parent->n_chaves++;

    write_node(t, y_idx, y);
    write_node(t, z_idx, z);
    write_node(t, parent_idx, parent);
    write_header(t);

    node_destroy(y);
    node_destroy(z);
}

static void insert_non_full(BTree *t, long idx,
                            BTreeNode *n,
                            int key, int reg)
{
    int i = n->n_chaves - 1;

    if (n->is_leaf)
    {

        while (i >= 0 && key < n->chaves[i])
        {
            n->chaves[i + 1] = n->chaves[i];
            n->registros[i + 1] = n->registros[i];
            i--;
        }

        n->chaves[i + 1] = key;
        n->registros[i + 1] = reg;
        n->n_chaves++;

        write_node(t, idx, n);
    }
    else
    {
        while (i >= 0 && key < n->chaves[i])
            i--;
        i++;

        BTreeNode *child = read_node(t, n->filhos[i]);

        if (node_is_full(child, t->header.ordem))
        {
            split_child(t, idx, n, i);
            node_destroy(child);
            n = read_node(t, idx);

            if (key > n->chaves[i])
                i++;
            child = read_node(t, n->filhos[i]);
        }

        insert_non_full(t, n->filhos[i], child, key, reg);
        node_destroy(child);
    }
}

void btree_insert(BTree *t, int chave, int registro)
{
    int tmp;
    if (btree_search(t, chave, &tmp))
        return;

    BTreeNode *root = read_node(t, t->header.raiz);

    if (node_is_full(root, t->header.ordem))
    {

        long new_root_idx = t->header.prox_no++;
        BTreeNode *new_root =
            node_create(t->header.ordem, 0);

        new_root->filhos[0] = t->header.raiz;
        t->header.raiz = new_root_idx;

        split_child(t, new_root_idx, new_root, 0);
        insert_non_full(t, new_root_idx,
                        new_root, chave, registro);

        write_node(t, new_root_idx, new_root);
        write_header(t);
        node_destroy(new_root);
    }
    else
    {
        insert_non_full(t, t->header.raiz,
                        root, chave, registro);
    }

    node_destroy(root);
}

static void remove_from_leaf(BTreeNode *n, int idx)
{
    for (int i = idx; i < n->n_chaves - 1; i++)
    {
        n->chaves[i] = n->chaves[i + 1];
        n->registros[i] = n->registros[i + 1];
    }
    n->n_chaves--;
}

static void remove_rec(BTree *t, long idx, int key)
{
    BTreeNode *n = read_node(t, idx);
    int i = 0;

    while (i < n->n_chaves && key > n->chaves[i])
        i++;

    if (i < n->n_chaves && key == n->chaves[i])
    {

        if (n->is_leaf)
        {
            remove_from_leaf(n, i);
            write_node(t, idx, n);
        }

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
    remove_rec(t, next, key);
}

void btree_remove(BTree *t, int chave)
{
    remove_rec(t, t->header.raiz, chave);
}


void btree_print(BTree *t, FILE *out)
{
    if (!t || t->header.raiz == -1)
        return;

    Queue *q = queue_create();
    queue_push(q, t->header.raiz);

    while (!queue_empty(q))
    {
        Queue *next = queue_create();

        while (!queue_empty(q))
        {
            long rrn = queue_pop(q);

            /* cria nó temporário */
            BTreeNode *node =
                node_create(t->header.ordem, 0);

            long offset =
                sizeof(BTreeHeader) +
                rrn * node_disk_size(t->header.ordem);

            node_read(t->fp, offset, node);

            fprintf(out, "[");

            for (int i = 0; i < node->n_chaves; i++)
            {
                fprintf(out, "key: %d, ",
                        node->chaves[i]);
            }

            fprintf(out, "]");

            if (!node->is_leaf)
            {
                for (int i = 0;
                     i <= node->n_chaves;
                     i++)
                {
                    if (node->filhos[i] != -1)
                        queue_push(next,
                                   node->filhos[i]);
                }
            }

            node_destroy(node);
        }

        fprintf(out, "\n");

        queue_destroy(q);
        q = next;
    }

    queue_destroy(q);
}
