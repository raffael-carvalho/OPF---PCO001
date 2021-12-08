#include "subgrafico.h"

/* ----------- Construtor e destruidor ------------------------ */
// Alocar nós sem atributos
Subgrafico *CriaSubgrafico(int nnodes)
{
  Subgrafico *sg = (Subgrafico *)calloc(1, sizeof(Subgrafico));
  int i;

  sg->nnodes = nnodes;
  sg->node = (SNode *)calloc(nnodes, sizeof(SNode));
  sg->ordered_list_of_nodes = (int *)calloc(nnodes, sizeof(int));

  if (sg->node == NULL)
  {
    Error("Não é possível alocar nós", "CriaSubgrafico");
  }

  sg->nnodes_adj = 0;

  for (i = 0; i < sg->nnodes; i++)
  {
    sg->node[i].atributos = NULL;
    sg->node[i].relevante = 0;
    sg->node[i].densidadeNo = 0.0;
  }

  return (sg);
}

//Desalocar memória para Subgrafico
void DestroiSubgrafico(Subgrafico **sg)
{
  int i;

  if ((*sg) != NULL)
  {
    for (i = 0; i < (*sg)->nnodes; i++)
    {
      if ((*sg)->node[i].atributos != NULL)
        free((*sg)->node[i].atributos);
      if ((*sg)->node[i].adj != NULL)
        DestroySet(&(*sg)->node[i].adj);
    }
    free((*sg)->node);
    free((*sg)->ordered_list_of_nodes);
    free((*sg));
    *sg = NULL;
  }
}

//escrever Subgrafico no disco
void EscreveSubgrafico(Subgrafico *g, char *file)
{
  FILE *fp = NULL;
  int i, j;

  if (g != NULL)
  {
    fp = fopen(file, "wb");
    fwrite(&g->nnodes, sizeof(int), 1, fp);
    fwrite(&g->nClasses, sizeof(int), 1, fp);
    fwrite(&g->nAtributos, sizeof(int), 1, fp);
    fprintf(stderr, "Amostras: %d\nClasses: %d\nAtributos: %d\n", g->nnodes, g->nClasses, g->nAtributos);

    /* posição de escrita (id), rótulo e atributos */
    for (i = 0; i < g->nnodes; i++)
    {
      fwrite(&g->node[i].posicao, sizeof(int), 1, fp);
      fwrite(&g->node[i].classeTrue, sizeof(int), 1, fp);
      for (j = 0; j < g->nAtributos; j++)
        fwrite(&g->node[i].atributos[j], sizeof(float), 1, fp);
    }
    fclose(fp);
  }
}

//leia o Subgrafico a partir do arquivo de formato opf
Subgrafico *LeSubgrafico(char *file)
{
  Subgrafico *g = NULL;
  FILE *fp = NULL;
  int nnodes, i, j;
  char msg[256];

  if ((fp = fopen(file, "rb")) == NULL)
  {
    sprintf(msg, "%s%s", "Impossivel abrir o arquivo", file);
    Error(msg, "LeSubgrafico");
  }
  /* lendo # de nós, classes e atributos*/
  if (fread(&nnodes, sizeof(int), 1, fp) != 1)
    Error("Nao foi possivel ler o numero de nos", "LeSubgrafico");
  g = CriaSubgrafico(nnodes);
  if (fread(&g->nClasses, sizeof(int), 1, fp) != 1)
    Error("Nao foi possivel ler o numero de classes", "LeSubgrafico");
  if (fread(&g->nAtributos, sizeof(int), 1, fp) != 1)
    Error("Nao foi possivel ler o numero de atributos", "LeSubgrafico");

  /* atributos de leitura */
  for (i = 0; i < g->nnodes; i++)
  {
    g->node[i].atributos = AllocFloatArray(g->nAtributos);
    if (fread(&g->node[i].posicao, sizeof(int), 1, fp) != 1)
      Error("Nao foi possivel ler a posicao do no", "LeSubgrafico");
    if (fread(&g->node[i].classeTrue, sizeof(int), 1, fp) != 1)
      Error("Nao foi possivel ler o classe verdadeiro do no", "LeSubgrafico");

    for (j = 0; j < g->nAtributos; j++)
      if (fread(&g->node[i].atributos[j], sizeof(float), 1, fp) != 1)
        Error("Nao foi possivel ler os atributos do no", "LeSubgrafico");
  }

  fclose(fp);

  return g;
}

//Copiar Subgrafico (não copia Arcs)
Subgrafico *CopiaSubgrafico(Subgrafico *g)
{
  Subgrafico *clone = NULL;
  int i;

  if (g != NULL)
  {
    clone = CriaSubgrafico(g->nnodes);

    clone->nnodes_adj = g->nnodes_adj;
    clone->df = g->df;
    clone->nClasses = g->nClasses;
    clone->nAtributos = g->nAtributos;
    clone->mindens = g->mindens;
    clone->maxdens = g->maxdens;
    clone->K = g->K;

    for (i = 0; i < g->nnodes-1; i++)
    {
      CopiaSNode(&clone->node[i], &g->node[i], g->nAtributos);
      clone->ordered_list_of_nodes[i] = g->ordered_list_of_nodes[i];
    }

    return clone;
  }
  else
    return NULL;
}

//Copia No
void CopiaSNode(SNode *dest, SNode *src, int nfeats)
{
  dest->atributos = AllocFloatArray(nfeats);
  memcpy(dest->atributos, src->atributos, nfeats * sizeof(float));
  dest->caminhoValor = src->caminhoValor;
  dest->densidadeNo = src->densidadeNo;
  dest->classe = src->classe;
  dest->raiz = src->raiz;
  dest->predecessor = src->predecessor;
  dest->classeTrue = src->classeTrue;
  dest->posicao = src->posicao;
  dest->status = src->status;
  dest->relevante = src->relevante;
  dest->raio = src->raio;
  dest->nplatadj = src->nplatadj;

  dest->adj = CloneSet(src->adj);
}

//Troca nós
void TrocaSNode(SNode *a, SNode *b)
{
  SNode tmp;

  tmp = *a;
  *a = *b;
  *b = tmp;
}
