#include "set.h"

void InsertSet(Set **S, int elem)
{
  Set *p = NULL;

  p = (Set *)calloc(1, sizeof(Set));
  if (p == NULL)
    Error(MSG1, "InsertSet");
  if (*S == NULL)
  {
    p->elem = elem;
    p->next = NULL;
  }
  else
  {
    p->elem = elem;
    p->next = *S;
  }
  *S = p;
}

int RemoveSet(Set **S)
{
  Set *p;
  int elem = NIL;

  if (*S != NULL)
  {
    p = *S;
    elem = p->elem;
    *S = p->next;
    free(p);
  }

  return (elem);
}

int GetSetSize(Set *S)
{
  Set *aux;
  int size = 0;

  aux = S;
  while (aux != NULL)
  {
    size++;
    aux = aux->next;
  }
  return size;
}

Set *CloneSet(Set *S)
{
  Set *tmp = NULL;
  Set *C = NULL;
  Set **tail = NULL;
  int p;

  tmp = S;

  if (tmp != NULL)
  {
    p = tmp->elem;
    C = (Set *)calloc(1, sizeof(Set));
    C->elem = p;
    C->next = NULL;
    tail = &(C->next);
    tmp = tmp->next;
  }

  while (tmp != NULL)
  {
    p = tmp->elem;
    *tail = (Set *)calloc(1, sizeof(Set));
    (*tail)->elem = p;
    (*tail)->next = NULL;
    tail = &((*tail)->next);
    tmp = tmp->next;
  }
  return C;
}

void DestroySet(Set **S)
{
  Set *p;
  while (*S != NULL)
  {
    p = *S;
    *S = p->next;
    free(p);
  }
}
