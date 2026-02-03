#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue Queue;

/* cria fila vazia */
Queue *queue_create(void);

/* libera a fila */
void queue_destroy(Queue *q);

/* insere no final */
void queue_push(Queue *q, long value);

/* remove do início (assume não vazia) */
long queue_pop(Queue *q);

/* retorna 1 se vazia, 0 caso contrário */
int queue_empty(Queue *q);

#endif
