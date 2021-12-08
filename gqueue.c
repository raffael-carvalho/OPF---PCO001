#include "gqueue.h"

GQueue *CreateGQueue(int nbuckets, int nelems, int *value)
{
    GQueue *Q = NULL;

    Q = (GQueue *)malloc(1 * sizeof(GQueue));

    if (Q != NULL)
    {
        Q->C.first = (int *)malloc((nbuckets + 1) * sizeof(int));
        Q->C.last = (int *)malloc((nbuckets + 1) * sizeof(int));
        Q->C.nbuckets = nbuckets;
        if ((Q->C.first != NULL) && (Q->C.last != NULL))
        {
            Q->L.elem = (GQNode *)malloc(nelems * sizeof(GQNode));
            Q->L.nelems = nelems;
            Q->L.value = value;
            if (Q->L.elem != NULL)
            {
                ResetGQueue(Q);
            }
            else
                Error(MSG1, "CreateGQueue");
        }
        else
            Error(MSG1, "CreateGQueue");
    }
    else
        Error(MSG1, "CreateGQueue");

    return (Q);
}

void ResetGQueue(GQueue *Q)
{
    int i;

    Q->C.minvalue = INT_MAX;
    Q->C.maxvalue = INT_MIN;
    SetTieBreak(Q, FIFOBREAK);
    SetRemovalPolicy(Q, MINVALUE);
    for (i = 0; i < Q->C.nbuckets + 1; i++)
        Q->C.first[i] = Q->C.last[i] = NIL;

    for (i = 0; i < Q->L.nelems; i++)
    {
        Q->L.elem[i].next = Q->L.elem[i].prev = NIL;
        Q->L.elem[i].color = WHITE;
    }
}

void DestroyGQueue(GQueue **Q)
{
    GQueue *aux;

    aux = *Q;
    if (aux != NULL)
    {
        if (aux->C.first != NULL)
            free(aux->C.first);
        if (aux->C.last != NULL)
            free(aux->C.last);
        if (aux->L.elem != NULL)
            free(aux->L.elem);
        free(aux);
        *Q = NULL;
    }
}

GQueue *GrowGQueue(GQueue **Q, int nbuckets)
{
    GQueue *Q1 = CreateGQueue(nbuckets, (*Q)->L.nelems, (*Q)->L.value);
    int i, bucket;

    Q1->C.minvalue = (*Q)->C.minvalue;
    Q1->C.maxvalue = (*Q)->C.maxvalue;
    Q1->C.tiebreak = (*Q)->C.tiebreak;
    Q1->C.removal_policy = (*Q)->C.removal_policy;
    for (i = 0; i < (*Q)->C.nbuckets; i++)
        if ((*Q)->C.first[i] != NIL)
        {
            bucket = (*Q)->L.value[(*Q)->C.first[i]] % Q1->C.nbuckets;
            Q1->C.first[bucket] = (*Q)->C.first[i];
            Q1->C.last[bucket] = (*Q)->C.last[i];
        }
    if ((*Q)->C.first[(*Q)->C.nbuckets] != NIL)
    {
        bucket = Q1->C.nbuckets;
        Q1->C.first[bucket] = (*Q)->C.first[(*Q)->C.nbuckets];
        Q1->C.last[bucket] = (*Q)->C.last[(*Q)->C.nbuckets];
    }

    for (i = 0; i < (*Q)->L.nelems; i++)
        Q1->L.elem[i] = (*Q)->L.elem[i];

    DestroyGQueue(Q);
    return (Q1);
}

void InsertGQueue(GQueue **Q, int elem)
{
    int bucket, minvalue = (*Q)->C.minvalue, maxvalue = (*Q)->C.maxvalue;

    if (((*Q)->L.value[elem] == INT_MAX) || ((*Q)->L.value[elem] == INT_MIN))
        bucket = (*Q)->C.nbuckets;
    else
    {
        if ((*Q)->L.value[elem] < minvalue)
            minvalue = (*Q)->L.value[elem];
        if ((*Q)->L.value[elem] > maxvalue)
            maxvalue = (*Q)->L.value[elem];
        if ((maxvalue - minvalue) > ((*Q)->C.nbuckets - 1))
        {
            (*Q) = GrowGQueue(Q, 2 * (maxvalue - minvalue) + 1);
            fprintf(stdout, "Aviso: dobrando o tamanho da fila\n");
        }
        if ((*Q)->C.removal_policy == MINVALUE)
        {
            bucket = (*Q)->L.value[elem] % (*Q)->C.nbuckets;
        }
        else
        {
            bucket = (*Q)->C.nbuckets - 1 - ((*Q)->L.value[elem] % (*Q)->C.nbuckets);
        }
        (*Q)->C.minvalue = minvalue;
        (*Q)->C.maxvalue = maxvalue;
    }
    if ((*Q)->C.first[bucket] == NIL)
    {
        (*Q)->C.first[bucket] = elem;
        (*Q)->L.elem[elem].prev = NIL;
    }
    else
    {
        (*Q)->L.elem[(*Q)->C.last[bucket]].next = elem;
        (*Q)->L.elem[elem].prev = (*Q)->C.last[bucket];
    }

    (*Q)->C.last[bucket] = elem;
    (*Q)->L.elem[elem].next = NIL;
    (*Q)->L.elem[elem].color = GRAY;
}

int RemoveGQueue(GQueue *Q)
{
    int elem = NIL, next, prev;
    int last, current;

    if (Q->C.removal_policy == MINVALUE)
        current = Q->C.minvalue % Q->C.nbuckets;
    else
        current = Q->C.nbuckets - 1 - (Q->C.maxvalue % Q->C.nbuckets);

    /** moves to next element **/

    if (Q->C.first[current] == NIL)
    {
        last = current;

        current = (current + 1) % (Q->C.nbuckets);

        while ((Q->C.first[current] == NIL) && (current != last))
        {
            current = (current + 1) % (Q->C.nbuckets);
        }

        if (Q->C.first[current] != NIL)
        {
            if (Q->C.removal_policy == MINVALUE)
                Q->C.minvalue = Q->L.value[Q->C.first[current]];
            else
                Q->C.maxvalue = Q->L.value[Q->C.first[current]];
        }
        else
        {
            if (Q->C.first[Q->C.nbuckets] != NIL)
            {
                current = Q->C.nbuckets;
                if (Q->C.removal_policy == MINVALUE)
                    Q->C.minvalue = Q->L.value[Q->C.first[current]];
                else
                    Q->C.maxvalue = Q->L.value[Q->C.first[current]];
            }
            else
            {
                Error("GQueue is empty\n", "RemoveGQueue");
            }
        }
    }

    if (Q->C.tiebreak == LIFOBREAK)
    {
        elem = Q->C.last[current];
        prev = Q->L.elem[elem].prev;
        if (prev == NIL) /* there was a single element in the list */
        {
            Q->C.last[current] = Q->C.first[current] = NIL;
        }
        else
        {
            Q->C.last[current] = prev;
            Q->L.elem[prev].next = NIL;
        }
    }
    else /* Assume FIFO policy for breaking ties */
    {
        elem = Q->C.first[current];
        next = Q->L.elem[elem].next;
        if (next == NIL) /* there was a single element in the list */
        {
            Q->C.first[current] = Q->C.last[current] = NIL;
        }
        else
        {
            Q->C.first[current] = next;
            Q->L.elem[next].prev = NIL;
        }
    }

    Q->L.elem[elem].color = BLACK;

    return elem;
}

void RemoveGQueueElem(GQueue *Q, int elem)
{
    int prev, next, bucket;

    if ((Q->L.value[elem] == INT_MAX) || (Q->L.value[elem] == INT_MIN))
        bucket = Q->C.nbuckets;
    else
    {
        if (Q->C.removal_policy == MINVALUE)
            bucket = Q->L.value[elem] % Q->C.nbuckets;
        else
            bucket = Q->C.nbuckets - 1 - (Q->L.value[elem] % Q->C.nbuckets);
    }

    prev = Q->L.elem[elem].prev;
    next = Q->L.elem[elem].next;

    /* if elem is the first element */
    if (Q->C.first[bucket] == elem)
    {
        Q->C.first[bucket] = next;
        if (next == NIL) /* elem is also the last one */
            Q->C.last[bucket] = NIL;
        else
            Q->L.elem[next].prev = NIL;
    }
    else /* elem is in the middle or it is the last */
    {
        Q->L.elem[prev].next = next;
        if (next == NIL) /* if it is the last */
            Q->C.last[bucket] = prev;
        else
            Q->L.elem[next].prev = prev;
    }

    Q->L.elem[elem].color = BLACK;
}

void UpdateGQueue(GQueue **Q, int elem, int newvalue)
{
    RemoveGQueueElem(*Q, elem);
    (*Q)->L.value[elem] = newvalue;
    InsertGQueue(Q, elem);
}

int EmptyGQueue(GQueue *Q)
{
    int last, current;

    if (Q->C.removal_policy == MINVALUE)
        current = Q->C.minvalue % Q->C.nbuckets;
    else
        current = Q->C.nbuckets - 1 - (Q->C.maxvalue % Q->C.nbuckets);

    if (Q->C.first[current] != NIL)
        return 0;

    last = current;

    current = (current + 1) % (Q->C.nbuckets);

    while ((Q->C.first[current] == NIL) && (current != last))
    {
        current = (current + 1) % (Q->C.nbuckets);
    }

    if (Q->C.first[current] == NIL)
    {
        if (Q->C.first[Q->C.nbuckets] == NIL)
        {
            return (1);
        }
    }

    return (0);
}
