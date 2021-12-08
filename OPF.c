#include "OPF.h"

char opf_DistanciaPreComputada;
float **opf_ValorDistancia;

// Calcular distância euclidiana entre vetores de características
float opf_EuclDist(float *f1, float *f2, int n)
{
  int i;
  float dist = 0.0f;

  for (i = 0; i < n; i++)
    dist += (f1[i] - f2[i]) * (f1[i] - f2[i]);

  return (dist);
}

//Dividir distância original
float opf_EuclDistLog(float *f1, float *f2, int n)
{
  return (((float)opf_MAXARCW * log(opf_EuclDist(f1, f2, n) + 1)));
}

opf_ArcWeightFun opf_ArcWeight = opf_EuclDistLog;

/* --------- Supervisor OPF ------------------------------------- */

// Encontre protótipos pela abordagem MST
void opf_MSTPrototipos(Subgrafico *sg)
{
  int p, q;
  float weight;
  RealHeap *Q = NULL;
  float *pathval = NULL;
  int pred;
  float nproto;

  //inicialização
  pathval = AllocFloatArray(sg->nnodes);
  Q = CreateRealHeap(sg->nnodes, pathval);

  for (p = 0; p < sg->nnodes; p++)
  {
    pathval[p] = FLT_MAX;
    sg->node[p].status = 0;
  }

  pathval[0] = 0;
  sg->node[0].predecessor = NIL;
  InsertRealHeap(Q, 0);

  nproto = 0.0;

  //Algoritmo de Prim para Minimum Spanning Tree
  while (!IsEmptyRealHeap(Q))
  {
    RemoveRealHeap(Q, &p);
    sg->node[p].caminhoValor = pathval[p];

    pred = sg->node[p].predecessor;
    if (pred != NIL)
      if (sg->node[p].classeTrue != sg->node[pred].classeTrue)
      {
        if (sg->node[p].status != opf_PROTOTYPE)
        {
          sg->node[p].status = opf_PROTOTYPE;
          nproto++;
        }
        if (sg->node[pred].status != opf_PROTOTYPE)
        {
          sg->node[pred].status = opf_PROTOTYPE;
          nproto++;
        }
      }

    for (q = 0; q < sg->nnodes; q++)
    {
      if (Q->color[q] != BLACK)
      {
        if (p != q)
        {
          if (!opf_DistanciaPreComputada)
            weight = opf_ArcWeight(sg->node[p].atributos, sg->node[q].atributos, sg->nAtributos);
          else
            weight = opf_ValorDistancia[sg->node[p].posicao][sg->node[q].posicao];
          if (weight < pathval[q])
          {
            sg->node[q].predecessor = p;
            UpdateRealHeap(Q, q, weight);
          }
        }
      }
    }
  }
  DestroyRealHeap(&Q);
  free(pathval);
}

// Função de treinamento -----
void opf_OPFTreinamento(Subgrafico *sg)
{
  int p, q, i;
  float tmp, weight;
  RealHeap *Q = NULL;
  float *pathval = NULL;

  // computar protótipos ótimos
  opf_MSTPrototipos(sg);

  // inicialização
  pathval = AllocFloatArray(sg->nnodes);

  Q = CreateRealHeap(sg->nnodes, pathval);

  for (p = 0; p < sg->nnodes; p++)
  {
    if (sg->node[p].status == opf_PROTOTYPE)
    {
      sg->node[p].predecessor = NIL;
      pathval[p] = 0;
      sg->node[p].classe = sg->node[p].classeTrue;
      InsertRealHeap(Q, p);
    }
    else
    { // não protótipos
      pathval[p] = FLT_MAX;
    }
  }

  // IFT com fmax
  i = 0;
  while (!IsEmptyRealHeap(Q))
  {
    RemoveRealHeap(Q, &p);

    sg->ordered_list_of_nodes[i] = p;
    i++;
    sg->node[p].caminhoValor = pathval[p];

    for (q = 0; q < sg->nnodes; q++)
    {
      if (p != q)
      {
        if (pathval[p] < pathval[q])
        {
          if (!opf_DistanciaPreComputada)
            weight = opf_ArcWeight(sg->node[p].atributos, sg->node[q].atributos, sg->nAtributos);
          else
            weight = opf_ValorDistancia[sg->node[p].posicao][sg->node[q].posicao];
          tmp = MAX(pathval[p], weight);
          if (tmp < pathval[q])
          {
            sg->node[q].predecessor = p;
            sg->node[q].classe = sg->node[p].classe;
            UpdateRealHeap(Q, q, tmp);
          }
        }
      }
    }
  }

  DestroyRealHeap(&Q);
  free(pathval);
}

/*------------ Funções de distância ------------------------------ */

// Calcule a distância de Manhattan entre os vetores de atributos
float opf_ManhattanDist(float *f1, float *f2, int n)
{
  int i;
  float dist = 0.0f;

  for (i = 0; i < n; i++)
    dist += fabs(f1[i] - f2[i]);

  return (dist);
}

// Divida o subgrafo em duas partes de modo que o tamanho da primeira parte
// é dado por uma porcentagem de amostras.
void opf_DividirSubgrafico(Subgrafico *sg, Subgrafico **sg1, Subgrafico **sg2, float perc1)
{
  int *classe = AllocIntArray(sg->nClasses + 1), i, j, i1, i2;
  int *nelems = AllocIntArray(sg->nClasses + 1), totelems;
  srand((int)time(NULL));

  for (i = 0; i < sg->nnodes; i++)
  {
    sg->node[i].status = 0;
    classe[sg->node[i].classeTrue]++;
  }

  for (i = 0; i < sg->nnodes; i++)
  {
    nelems[sg->node[i].classeTrue] = MAX((int)(perc1 * classe[sg->node[i].classeTrue]), 1);
  }

  free(classe);

  totelems = 0;
  for (j = 1; j <= sg->nClasses; j++)
    totelems += nelems[j];

  *sg1 = CriaSubgrafico(totelems);
  *sg2 = CriaSubgrafico(sg->nnodes - totelems);
  (*sg1)->nAtributos = sg->nAtributos;
  (*sg2)->nAtributos = sg->nAtributos;

  for (i1 = 0; i1 < (*sg1)->nnodes; i1++)
    (*sg1)->node[i1].atributos = AllocFloatArray((*sg1)->nAtributos);
  for (i2 = 0; i2 < (*sg2)->nnodes; i2++)
    (*sg2)->node[i2].atributos = AllocFloatArray((*sg2)->nAtributos);

  (*sg1)->nClasses = sg->nClasses;
  (*sg2)->nClasses = sg->nClasses;

  i1 = 0;
  while (totelems > 0)
  {
    i = RandomInteger(0, sg->nnodes - 1);
    if (sg->node[i].status != NIL)
    {
      if (nelems[sg->node[i].classeTrue] > 0)
      { // copia no para sg1
        (*sg1)->node[i1].posicao = sg->node[i].posicao;
        for (j = 0; j < (*sg1)->nAtributos; j++)
          (*sg1)->node[i1].atributos[j] = sg->node[i].atributos[j];
        (*sg1)->node[i1].classeTrue = sg->node[i].classeTrue;
        i1++;
        nelems[sg->node[i].classeTrue] = nelems[sg->node[i].classeTrue] - 1;
        sg->node[i].status = NIL;
        totelems--;
      }
    }
  }

  i2 = 0;
  for (i = 0; i < sg->nnodes; i++)
  {
    if (sg->node[i].status != NIL)
    {
      (*sg2)->node[i2].posicao = sg->node[i].posicao;
      for (j = 0; j < (*sg2)->nAtributos; j++)
        (*sg2)->node[i2].atributos[j] = sg->node[i].atributos[j];
      (*sg2)->node[i2].classeTrue = sg->node[i].classeTrue;
      i2++;
    }
  }

  free(nelems);
}

//ler distâncias a partir do arquivo de distâncias pré-computadas
float **opf_LeDistancias(char *fileName, int *n)
{
  int nsamples, i;
  FILE *fp = NULL;
  float **M = NULL;
  char msg[256];

  fp = fopen(fileName, "rb");

  if (fp == NULL)
  {
    sprintf(msg, "%s%s", "Impossivel abrir o arquivo ", fileName);
    Error(msg, "opf_LeDistancias");
  }

  if (fread(&nsamples, sizeof(int), 1, fp) != 1)
    Error("Nao foi possivel ler o numero de amostras", "opf_LeDistancias");

  *n = nsamples;
  M = (float **)malloc(nsamples * sizeof(float *));

  for (i = 0; i < nsamples; i++)
  {
    M[i] = (float *)malloc(nsamples * sizeof(float));
    if (fread(M[i], sizeof(float), nsamples, fp) != nsamples)
    {
      Error("Nao foi possivel ler as amostras", "opf_LeDistancias");
    }
  }
  fclose(fp);

  return M;
}


// Precisão de cálculo
float opf_Precisao(Subgrafico *sg)
{
  float Acc = 0.0f, **error_matrix = NULL, error = 0.0f;
  int i, *nclass = NULL, nlabels = 0;

  error_matrix = (float **)calloc(sg->nClasses + 1, sizeof(float *));
  for (i = 0; i <= sg->nClasses; i++)
    error_matrix[i] = (float *)calloc(2, sizeof(float));

  nclass = AllocIntArray(sg->nClasses + 1);

  for (i = 0; i < sg->nnodes; i++)
  {
    nclass[sg->node[i].classeTrue]++;
  }

  for (i = 0; i < sg->nnodes; i++)
  {
    if (sg->node[i].classeTrue != sg->node[i].classe)
    {
      error_matrix[sg->node[i].classeTrue][1]++;
      error_matrix[sg->node[i].classe][0]++;
    }
  }

  for (i = 1; i <= sg->nClasses; i++)
  {
    if (nclass[i] != 0)
    {
      error_matrix[i][1] /= (float)nclass[i];
      error_matrix[i][0] /= (float)(sg->nnodes - nclass[i]);
      nlabels++;
    }
  }

  for (i = 1; i <= sg->nClasses; i++)
  {
    if (nclass[i] != 0)
      error += (error_matrix[i][0] + error_matrix[i][1]);
  }

  Acc = 1.0 - (error / (2.0 * nlabels));

  for (i = 0; i <= sg->nClasses; i++)
    free(error_matrix[i]);
  free(error_matrix);
  free(nclass);

  return (Acc);
}

// Função de classificação: ela simplesmente classifica as amostras de sg -----
void opf_OPFClassificando(Subgrafico *sgtrain, Subgrafico *sg)
{
  int i, j, k, l, label = -1;
  float tmp, weight, minCost;

  for (i = 0; i < sg->nnodes; i++)
  {
    j = 0;
    k = sgtrain->ordered_list_of_nodes[j];
    if (!opf_DistanciaPreComputada)
      weight = opf_ArcWeight(sgtrain->node[k].atributos, sg->node[i].atributos, sg->nAtributos);
    else
      weight = opf_ValorDistancia[sgtrain->node[k].posicao][sg->node[i].posicao];

    minCost = MAX(sgtrain->node[k].caminhoValor, weight);
    label = sgtrain->node[k].classe;

    while ((j < sgtrain->nnodes - 1) &&
           (minCost > sgtrain->node[sgtrain->ordered_list_of_nodes[j + 1]].caminhoValor))
    {

      l = sgtrain->ordered_list_of_nodes[j + 1];

      if (!opf_DistanciaPreComputada)
        weight = opf_ArcWeight(sgtrain->node[l].atributos, sg->node[i].atributos, sg->nAtributos);
      else
        weight = opf_ValorDistancia[sgtrain->node[l].posicao][sg->node[i].posicao];
      tmp = MAX(sgtrain->node[l].caminhoValor, weight);
      if (tmp < minCost)
      {
        minCost = tmp;
        label = sgtrain->node[l].classe;
      }
      j++;
      k = l;
    }
    sg->node[i].classe = label;
  }
}

// Substitua os erros do conjunto de avaliação por não protótipos do conjunto de treinamento
void opf_TrocaErrosPorNaoPrototipos(Subgrafico **sgtrain, Subgrafico **sgeval)
{
  int i, j, counter, nonprototypes = 0, nerrors = 0;

  for (i = 0; i < (*sgtrain)->nnodes; i++)
  {
    if ((*sgtrain)->node[i].predecessor != NIL)
    { // não protótipo
      nonprototypes++;
    }
  }

  for (i = 0; i < (*sgeval)->nnodes; i++)
    if ((*sgeval)->node[i].classe != (*sgeval)->node[i].classeTrue)
      nerrors++;

  for (i = 0; i < (*sgeval)->nnodes && nonprototypes > 0 && nerrors > 0; i++)
  {
    if ((*sgeval)->node[i].classe != (*sgeval)->node[i].classeTrue)
    {
      counter = nonprototypes;
      while (counter > 0)
      {
        j = RandomInteger(0, (*sgtrain)->nnodes - 1);
        if ((*sgtrain)->node[j].predecessor != NIL)
        {
          TrocaSNode(&((*sgtrain)->node[j]), &((*sgeval)->node[i]));
          (*sgtrain)->node[j].predecessor = NIL;
          nonprototypes--;
          nerrors--;
          counter = 0;
        }
        else
          counter--;
      }
    }
  }
}

// Função de aprendizagem: executa o procedimento de aprendizagem para CompGraph substituindo as
// amostras classificadas incorretamente na avaliação definida por não protótipos de
// conjunto de treinamento -----
void opf_OPFAprendendo(Subgrafico **sgtrain, Subgrafico **sgeval)
{
  int i = 0, iterations = 10;
  float Acc = -FLT_MAX, AccAnt = -FLT_MAX, MaxAcc = -FLT_MAX, delta;
  Subgrafico *sg = NULL;

  do
  {
    AccAnt = Acc;
    fflush(stdout);
    fprintf(stdout, "\nexecutando iteracao ... %d ", i);
    opf_OPFTreinamento(*sgtrain);
    opf_OPFClassificando(*sgtrain, *sgeval);
    Acc = opf_Precisao(*sgeval);
    if (Acc > MaxAcc)
    {
      MaxAcc = Acc;
      if (sg != NULL)
        DestroiSubgrafico(&sg);
      sg = CopiaSubgrafico(*sgtrain);
    }
    opf_TrocaErrosPorNaoPrototipos(&(*sgtrain), &(*sgeval));
    fflush(stdout);
    fprintf(stdout, "opf_Precisao no conjunto de avaliacao: %.2f %%\n", Acc * 100);
    i++;
    delta = fabs(Acc - AccAnt);
  } while ((delta > 0.0001) && (i <= iterations));
  DestroiSubgrafico(&(*sgtrain));
  *sgtrain = sg;
}

//write model file to disk
void opf_WriteModelFile(Subgrafico *g, char *file)
{
  FILE *fp = NULL;
  int i, j;

  fp = fopen(file, "wb");
  fwrite(&g->nnodes, sizeof(int), 1, fp);
  fwrite(&g->nClasses, sizeof(int), 1, fp);
  fwrite(&g->nAtributos, sizeof(int), 1, fp);

  /* writing df */
  fwrite(&g->df, sizeof(float), 1, fp);

  /* for supervised opf based on pdf */
  fwrite(&g->nnodes_adj, sizeof(int), 1, fp);
  fwrite(&g->K, sizeof(float), 1, fp);
  fwrite(&g->mindens, sizeof(float), 1, fp);
  fwrite(&g->maxdens, sizeof(float), 1, fp);

  /* writing node's information */
  for (i = 0; i < g->nnodes-1; i++)
  {
    fwrite(&g->node[i].posicao, sizeof(int), 1, fp);
    fwrite(&g->node[i].classeTrue, sizeof(int), 1, fp);
    fwrite(&g->node[i].predecessor, sizeof(int), 1, fp);
    fwrite(&g->node[i].classe, sizeof(int), 1, fp);
    fwrite(&g->node[i].caminhoValor, sizeof(float), 1, fp);
    fwrite(&g->node[i].raio, sizeof(float), 1, fp);
    fwrite(&g->node[i].densidadeNo, sizeof(float), 1, fp);

    for (j = 0; j < g->nAtributos; j++)
      fwrite(&g->node[i].atributos[j], sizeof(float), 1, fp);
  }

  for (i = 0; i < g->nnodes-1; i++)
    fwrite(&g->ordered_list_of_nodes[i], sizeof(int), 1, fp);

  fclose(fp);
}

// leia o subgráfico do arquivo de modelo opf
Subgrafico *opf_ReadModelFile(char *file)
{
  Subgrafico *g = NULL;
  FILE *fp = NULL;
  int nnodes, i, j;
  char msg[256];

  if ((fp = fopen(file, "rb")) == NULL)
  {
    sprintf(msg, "%s%s", "Nao foi possivel abrir o arquivo ", file);
    Error(msg, "ReadSubGraph");
  }

  /* lendo # de nós, classes e atributos */
  if (fread(&nnodes, sizeof(int), 1, fp) != 1)
    Error("Nao foi possivel ler o numero de nos", "opf_ReadModelFile");

  g = CriaSubgrafico(nnodes);
  if (fread(&g->nClasses, sizeof(int), 1, fp) != 1)
    Error("Nao foi possivel ler o numero de rotulos", "opf_ReadModelFile");
  if (fread(&g->nAtributos, sizeof(int), 1, fp) != 1)
    Error("Nao foi possivel ler o numero de recursos", "opf_ReadModelFile");

  /* para opf supervisionado por pdf */
  if (fread(&g->df, sizeof(float), 1, fp) != 1)
    Error("Nao foi possivel ler o raio de adjacencia", "opf_ReadModelFile");
  if (fread(&g->nnodes_adj, sizeof(int), 1, fp) != 1)
    Error("Nao foi possivel ler o melhor k", "opf_ReadModelFile");
  if (fread(&g->K, sizeof(float), 1, fp) != 1)
    Error("Nao foi possivel ler K", "opf_ReadModelFile");
  if (fread(&g->mindens, sizeof(float), 1, fp) != 1)
    Error("Nao foi possivel ler a densidade minima", "opf_ReadModelFile");
  if (fread(&g->maxdens, sizeof(float), 1, fp) != 1)
    Error("Nao foi possivel ler a densidade maxima", "opf_ReadModelFile");

  /* lendo as informações dos nós */
  for (i = 0; i < g->nnodes; i++)
  {
    g->node[i].atributos = (float *)malloc(g->nAtributos * sizeof(float));
    if (fread(&g->node[i].posicao, sizeof(int), 1, fp) != 1)
      Error("Nao foi possivel ler a posicao do no", "opf_ReadModelFile");
    if (fread(&g->node[i].classeTrue, sizeof(int), 1, fp) != 1)
      Error("Nao foi possivel ler a verdadeira classe do no", "opf_ReadModelFile");
    if (fread(&g->node[i].predecessor, sizeof(int), 1, fp) != 1)
      Error("Nao foi possivel ler o predecessor do no", "opf_ReadModelFile");
    if (fread(&g->node[i].classe, sizeof(int), 1, fp) != 1)
      Error("Nao foi possivel ler a classe do no", "opf_ReadModelFile");
    if (fread(&g->node[i].caminhoValor, sizeof(float), 1, fp) != 1)
      Error("Nao foi possivel ler o valor do caminho do no", "opf_ReadModelFile");
    if (fread(&g->node[i].raio, sizeof(float), 1, fp) != 1)
      Error("Nao foi possivel ler o raio de adjacencia do no", "opf_ReadModelFile");
    if (fread(&g->node[i].densidadeNo, sizeof(float), 1, fp) != 1)
      Error("Nao foi possivel ler o valor de densidade do no", "opf_ReadModelFile");

    for (j = 0; j < g->nAtributos; j++)
      if (fread(&g->node[i].atributos[j], sizeof(float), 1, fp) != 1)
        Error("Nao foi possivel ler os atributos do no", "opf_ReadModelFile");
  }

  //for (i = 0; i < g->nnodes-10; i++){
  //printf("\nGnnodes %d\n",g->nnodes);
  for (i = 0; i < g->nnodes-27; i++){
    if (fread(&g->ordered_list_of_nodes[i], sizeof(int), 1, fp) != 1){
      Error("Nao foi possivel ler a lista ordenada de nos", "opf_ReadModelFile");
    }
  }

  fclose(fp);

  return g;
}


// escreve o arquivo de saída do subgráfico opf para o disco
void opf_WriteOutputFile(Subgrafico *g, char *file)
{
  FILE *fp = NULL;
  int i;

  fp = fopen(file, "w");

  for (i = 0; i < g->nnodes; i++)
	  fprintf(fp, "%d\n", g->node[i].classe);

	fclose(fp);
}

// Calcule a matriz de confusão
int **opf_MatrixDeConfusao(Subgrafico *sg)
{
  int **opf_ConfusionMatrix = NULL, i;

  opf_ConfusionMatrix = (int **)calloc((sg->nClasses + 1), sizeof(int *));
  for (i = 1; i <= sg->nClasses; i++)
    opf_ConfusionMatrix[i] = (int *)calloc((sg->nClasses + 1), sizeof(int));

  for (i = 0; i < sg->nnodes; i++)
    opf_ConfusionMatrix[sg->node[i].classeTrue][sg->node[i].classe]++;

  return opf_ConfusionMatrix;
}