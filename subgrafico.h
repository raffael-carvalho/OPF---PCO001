#ifndef _SUBGRAFICO_H_
#define _SUBGRAFICO_H_

#include "common.h"
#include "set.h"

/*--------- Data types ----------------------------- */
typedef struct _snode {
  float caminhoValor; //valor do caminho
  float densidadeNo;    //densidade do nó
  float raio;   // distância máxima entre os k-vizinhos mais próximos em
		  // o conjunto de treinamento. É usado para propagar
		  // agrupando rótulos para nós de teste
  int   classe;   //rótulo do nó
  int   raiz;    //root node
  int   predecessor;    //no predecessor
  int   classeTrue; //rótulo verdadeiro se for conhecido
  int   posicao;  //indexar no espaço de atributos
  float *atributos;    //vetor de atributos
  char  status;  //0 - nada, 1 - prototipo
  char  relevante; //0 - irrelevante, 1 - relevante

  int nplatadj; // mantém a quantidade de nós adjacentes nos planaltos
                // É usado para otimizar opf_BestkMinCut
                // porque um gráfico knn precisa apenas ser construído
                // para kmax, mas a computação opf_PDF e opf_NormalizedCut
                // só precisa ser feito para o k atual,
                // ou até que k + nplatadj seja alcançado.
  Set   *adj;    //lista de adjacência para gráficos knn
} SNode;

typedef struct _subgraph {
  SNode *node;   //nós do subgrafo
  int   nnodes;  //número de nós
  int   nAtributos;  //número de atributos
  int   nnodes_adj;   //número de nós adjacentes
  int   nClasses; //numero de rotúlos 
  float df;      //raio no espaço do atributo para cálculo de densidade
  float mindens; //valor mínimo de densidade
  float maxdens; //valor de densidade máxima
  float K;       //Constante para cálculo opf_PDF
  int  *ordered_list_of_nodes; //Armazena a lista de nós em ordem crescente de custo para acelerar a classificação supervisionada.
} Subgrafico;

/* ----------- Construtor e destruidor ------------------------ */
Subgrafico *CriaSubgrafico(int nnodes); //Aloca nós sem atributos
void DestroiSubgrafico(Subgrafico **sg); //Desaloca memória para o subgrafo

void EscreveSubgrafico(Subgrafico *g, char *file); //escreve o subgráfico no disco
Subgrafico *LeSubgrafico(char *file);//lê o subgraph do arquivo de formato opf
Subgrafico *CopiaSubgrafico(Subgrafico *g);//Copiar subgrafico

void CopiaSNode(SNode *dest, SNode *src, int nfeats); //Copiar nós
void TrocaSNode(SNode *a, SNode *b); //Trocar nós
#endif // _SUBGRAFICO_H_
