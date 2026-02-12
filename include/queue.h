#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue Queue;

Queue *queue_create(void);
void   queue_destroy(Queue *q);
int    queue_empty(Queue *q);
void   queue_push(Queue *q, long value);
long   queue_pop(Queue *q);

#endif
