#include <stdlib.h>
#include "OPF.h"
#include "OPF.c"
#include <stdio.h>

void txt2opf(char *txt, char *opf)
{
	fprintf(stderr, "\nConvertendo arquivo .txt ASCII OPF para o formato binario OPF..");

	FILE *fpIn = NULL, *fpOut = NULL;
	int n, nfeats, nclasses, i, j, label;
	int id;
	float aux;

	fpIn = fopen(txt, "r");
	fpOut = fopen(opf, "wb");

	/* escrevendo o número de amostras */
	if (fscanf(fpIn, "%d", &n) != 1)
	{
		fprintf(stderr, "\n Nao foi possivel ler o numero de amostras");
		exit(-1);
	}
	fprintf(stderr, "\n Numero de amostras: %d", n);
	fwrite(&n, sizeof(int), 1, fpOut);

	/* escrevendo o número de classes */
	if (fscanf(fpIn, "%d", &nclasses) != 1)
	{
		fprintf(stderr, "\n Nao foi possivel ler o numero de classes \n");
		exit(-1);
	}

	fprintf(stderr, "\n numero de classes: %d", nclasses);
	fwrite(&nclasses, sizeof(int), 1, fpOut);

	/* escrevendo o número de atributos */
	if (fscanf(fpIn, "%d", &nfeats) != 1)
	{
		fprintf(stderr, "\n Nao foi possivel ler o numero de atributos \n");
		exit(-1);
	}

	fprintf(stderr, "\n numero de atributos: %d", nfeats);
	fwrite(&nfeats, sizeof(int), 1, fpOut);

	/* gravação de dados */
	for (i=0;i<n;i++)
	{
		if (fscanf(fpIn, "%d", &id) != 1)
		{
			fprintf(stderr, "\n Nao foi possivel ler o id da amostra na linha %d\n", &id);
			exit(-1);
		}
		fwrite(&id, sizeof(int), 1, fpOut);

		if (fscanf(fpIn, "%d", &label) != 1)
		{
			fprintf(stderr, "\n Nao foi possivel ler a classe da amostra na linha %d \n", i);
			exit(-1);
		}
		fwrite(&label, sizeof(int), 1, fpOut);

		for (j = 0; j < nfeats; j++)
		{
			if (fscanf(fpIn, "%f", &aux) != 1)
			{
				fprintf(stderr, "\n Nao foi possível ler os atributos de amostra na linha %d, atributo %d  \n", i, j);
				exit(-1);
			}

			fwrite(&aux, sizeof(float), 1, fpOut);
		}
	}
	printf("\nFinal loop");
	fclose(fpIn);
	fclose(fpOut);
    fprintf(stderr, "\nConversao realizada com sucesso!\n");
}

int main()
{
	fflush(stdout);
	fprintf(stdout, "\nProjeto: Classificacao baseada em Grafos");
	fprintf(stdout, "\nClassificacao por Floresta de Caminhos Otimos");
	fprintf(stdout, "\nOptimum-Path Forest (OPF)");
	fprintf(stdout, "\nPOSCOMP");
	fprintf(stdout, "\nUNIFEI");
	fprintf(stdout, "\n");
	fflush(stdout);

    fprintf(stderr, "\nPASSO 1: DEFINICAO DAS DISTANCIAS ENTRE OS VERTICES");
	fprintf(stderr, "\nSera considerada a distancia Manhattan (L1)\n");
    int dataset_n=0;
    while(dataset_n!=1&&dataset_n!=2&&dataset_n!=3){
        fprintf(stderr, "\nEscolha o dataset:");
        fprintf(stderr, "\nDigite 1 para Banana dataset;");
        fprintf(stderr, "\nDigite 2 para Iris dataset;");
        fprintf(stderr, "\nDigite 3 para Diabetic Retinopathy Debrecen dataset.\n");
        scanf("%d",&dataset_n);
    }
    Subgrafico *sg;
    char *dataset_escolhido;
	switch (dataset_n)
	{
	case 1:
		fprintf(stdout, "\nBanana dataset selecionado!");
        txt2opf("./dataset_txt/banana.txt", "./dataset_dat/banana.dat");
        sg = LeSubgrafico("./dataset_dat/banana.dat");
        dataset_escolhido = "./dataset_dat/banana.dat";
		break;
	case 2:
		fprintf(stdout, "\nIris dataset selecionado!");
        txt2opf("./dataset_txt/iris.txt", "./dataset_dat/iris.dat");
        sg = LeSubgrafico("./dataset_dat/iris.dat");
        dataset_escolhido = "./dataset_dat/iris.dat";
		break;
	case 3:
		fprintf(stdout, "\nDiabetic Retinopathy Debrecen dataset selecionado!");
        txt2opf("./dataset_txt/diabetic.txt", "./dataset_dat/diabetic.dat");
        sg = LeSubgrafico("./dataset_dat/diabetic.dat");
        dataset_escolhido = "./dataset_dat/diabetic.dat";
		break;
	default:
		fprintf(stderr, "\nOpcao dataset invalida!\n");
        return 0;
	}
	FILE *fp = fopen("./dataset_dat/distancias.dat", "wb");
	int i, j;
	float **Distancias = NULL, max = -FLT_MAX;

	fwrite(&sg->nnodes, sizeof(int), 1, fp);

	Distancias = (float **)malloc(sg->nnodes * sizeof(float *));
	for (i = 0; i < sg->nnodes; i++)
		Distancias[i] = (float *)malloc(sg->nnodes * sizeof(int));

	fprintf(stdout, "\n	Calculando distacia Manhattan (L1)...\n");
	for (i = 0; i < sg->nnodes-1; i++)
	{
		for (j = 0; j < sg->nnodes-1; j++)
		{
			if (i == j)
				Distancias[i][j] = 0.0;
			else
				Distancias[sg->node[i].posicao][sg->node[j].posicao] = opf_ManhattanDist(sg->node[i].atributos, sg->node[j].atributos, sg->nAtributos);
			if (Distancias[sg->node[i].posicao][sg->node[j].posicao] > max)
				max = Distancias[sg->node[i].posicao][sg->node[j].posicao];
		}
	}
    max = 1.0;
	for(i = 0; i < sg->nnodes; i++)
	{
		for (j = 0; j < sg->nnodes; j++)
		{
			Distancias[i][j] /= max;
			fwrite(&Distancias[i][j], sizeof(float), 1, fp);
		}
	}
	fprintf(stdout, "\nDistancias geradas ...\n");
	fflush(stdout);
	fprintf(stdout, "\nDesalocando memoria ...\n");
	for (i = 0; i < sg->nnodes; i++)
		free(Distancias[i]);
	free(Distancias);

	DestroiSubgrafico(&sg);
	fclose(fp);
    fprintf(stdout, "\nPASSO 1 FINALIZADO!\n");

	fflush(stdout);
	fprintf(stdout, "\nPASSO 2: DIVIDIR O CONJUNTO DE DADOS EM CONJUNTOS DE:");
	fprintf(stdout, "\nTREINAMENTO (30%%)");
	fprintf(stdout, "\nAVALIACAO (20%%)");
	fprintf(stdout, "\nTESTE (50%%)\n");
	fflush(stdout);

	Subgrafico *g = NULL, *gAux = NULL, *gTreinamento = NULL, *gAvaliacao = NULL, *gTeste = NULL;
	float treinamento_p = 0.3, avaliacao_p = 0.2, teste_p = 0.5;

	fprintf(stdout, "\nRecarregando dataset...");
	fflush(stdout);
	g = LeSubgrafico(dataset_escolhido);
	fprintf(stdout, " OK");
	fflush(stdout);

	fprintf(stdout, "\nDividindo dataset...");
	fflush(stdout);
	opf_DividirSubgrafico(g, &gAux, &gTeste, treinamento_p + avaliacao_p);

	if (avaliacao_p > 0)
		opf_DividirSubgrafico(gAux, &gTreinamento, &gAvaliacao, treinamento_p / (treinamento_p + avaliacao_p));
	else
		gTreinamento = CopiaSubgrafico(gAux);

	fprintf(stdout, " OK");
	fflush(stdout);

	fprintf(stdout, "\nGravando conjuntos de dados no disco...\n");
	fflush(stdout);
    fprintf(stdout, "\nDataset para treinamento:\n");
	EscreveSubgrafico(gTreinamento, "./dataset_dat/treinamento.dat");
    fprintf(stdout, "\nDataset para avaliacao:\n");
	EscreveSubgrafico(gAvaliacao, "./dataset_dat/avaliacao.dat");
    fprintf(stdout, "\nDataset para teste:\n");
	EscreveSubgrafico(gTeste, "./dataset_dat/teste.dat");
	fprintf(stdout, "\nPASSO 2 FINALIZADO!\n");
	fflush(stdout);

	fprintf(stdout, "\nDesalocando memoria ...");
	DestroiSubgrafico(&g);
	DestroiSubgrafico(&gAux);
	DestroiSubgrafico(&gTreinamento);
	DestroiSubgrafico(&gAvaliacao);
	DestroiSubgrafico(&gTeste);
	fprintf(stdout, " OK\n");

    fprintf(stderr, "\nPASSO 3: PROCEDIMENTO DE APRENDIZAGEM OPF");

	float Acc, time;
	char fileName[512];
	int n;
	timer tic;
	timer toc;
	FILE *f = NULL;


	fprintf(stdout, "\nLendo arquivo de dados...");
	fflush(stdout);
	Subgrafico *gTrain = LeSubgrafico("./dataset_dat/treinamento.dat"), *gEval = LeSubgrafico("./dataset_dat/avaliacao.dat");
	fprintf(stdout, " OK");
	fflush(stdout);

    opf_ValorDistancia = opf_LeDistancias("./dataset_dat/distancias.dat", &n);

	fprintf(stdout, "\nAprendendo com os erros no conjunto de avaliacao...");
	fflush(stdout);
	gettimeofday(&tic, NULL);
	opf_OPFAprendendo(&gTrain, &gEval);
	gettimeofday(&toc, NULL);
	time = ((toc.tv_sec - tic.tv_sec) * 1000.0 + (toc.tv_usec - tic.tv_usec) * 0.001) / 1000.0;
	Acc = opf_Precisao(gTrain);
	fprintf(stdout, "\nFinal opf_Precisao no conjunto de treinamento: %.2f%%", Acc * 100);
	fflush(stdout);
	Acc = opf_Precisao(gEval);
	fprintf(stdout, "\nFinal opf_Precisao no conjunto de avaliacaoo: %.2f%%", Acc * 100);
	fflush(stdout);

	fprintf(stdout, "\n\nGravando o arquivo de modelo do classificador ...");
	fflush(stdout);
	opf_WriteModelFile(gTrain, "./dataset_dat/classificador.opf");
	fprintf(stdout, " OK");
	fflush(stdout);

	fprintf(stdout, "\nDesalocando memoria ...");
	DestroiSubgrafico(&gTrain);
	DestroiSubgrafico(&gEval);
	for (i = 0; i < n; i++)
		free(opf_ValorDistancia[i]);
	free(opf_ValorDistancia);
	fprintf(stdout, " OK\n");
	fflush(stdout);

	sprintf(fileName, "%s.time", "./dataset_dat/distancias.dat");
	f = fopen(fileName, "a");
	fprintf(f, "%f\n", time);
	fclose(f);
    fprintf(stdout, "\nPASSO 3 FINALIZADO!\n");


    fprintf(stderr, "\nPASSO 4: CLASSIFICAR O CONJUNTO DE TESTE");


	fprintf(stdout, "\nLendo arquivos de dados ...");
	fflush(stdout);
	Subgrafico *gTest2 = LeSubgrafico("./dataset_dat/teste.dat"), *gTrain2 = opf_ReadModelFile("./dataset_dat/classificador.opf");
	fprintf(stdout, " OK");
	fflush(stdout);

	opf_ValorDistancia = opf_LeDistancias("./dataset_dat/distancias.dat", &n);

	fprintf(stdout, "\nClassificando conjunto de teste ...");
	fflush(stdout);
	gettimeofday(&tic, NULL);
	opf_OPFClassificando(gTrain2, gTest2);
	gettimeofday(&toc, NULL);
	fprintf(stdout, " OK");
	fflush(stdout);

	fprintf(stdout, "\nGravando arquivo de saida ...");
	fflush(stdout);
	sprintf(fileName, "%s.out", "./dataset_dat/teste.dat");
	opf_WriteOutputFile(gTest2, fileName);
	fprintf(stdout, " OK");
	fflush(stdout);

	fprintf(stdout, "\nDesalocando memoria ...");
	DestroiSubgrafico(&gTrain2);
	DestroiSubgrafico(&gTest2);
	for (i = 0; i < n; i++)
		free(opf_ValorDistancia[i]);
	free(opf_ValorDistancia);
	fprintf(stdout, " OK\n");

	time = ((toc.tv_sec - tic.tv_sec) * 1000.0 + (toc.tv_usec - tic.tv_usec) * 0.001) / 1000.0;
	//fprintf(stdout, "\nTempo de teste: %f segundos\n", time);
	fflush(stdout);

	sprintf(fileName, "%s.time", "./dataset_dat/teste.dat");
	f = fopen(fileName, "a");
	fprintf(f, "%f\n", time);
	fclose(f);
	fprintf(stdout, "\nPASSO 4 FINALIZADO!\n");

	fprintf(stderr, "\nPASSO 5: CALCULANDO A PRECISAO DO CONJUNTO DE TESTE");
	int **MC = NULL;
	float tmp;

	fprintf(stdout, "\nLendo arquivo de dados ...");
	fflush(stdout);
	Subgrafico *g2 = LeSubgrafico("./dataset_dat/teste.dat");
	fprintf(stdout, " OK");
	fflush(stdout);

	fprintf(stdout, "\nLendo arquivo de saida ...");
	fflush(stdout);
	sprintf(fileName, "%s.out", "./dataset_dat/teste.dat");
	f = fopen(fileName, "r");
	if (!f)
	{
		fprintf(stderr, "\nImpossivel abrir o arquivo %s", "./dataset_dat/teste.dat");
		exit(-1);
	}
	for (i = 0; i < g2->nnodes; i++)
		if (fscanf(f, "%d", &g2->node[i].classe) != 1)
		{
			fprintf(stderr, "\nErro ao ler a classe do no");
			exit(-1);
		}
	fclose(f);
	fprintf(stdout, " OK");
	fflush(stdout);

	MC = opf_MatrixDeConfusao(g2);
	for (i = 1; i <= g2->nClasses; i++)
	{
		fprintf(stderr, "\n");
		tmp = 0;
		for (j = 1; j <= g2->nClasses; j++)
		{
			tmp += MC[i][j];
			fprintf(stderr, "MC[%d][%d]: %d	", i, j, MC[i][j]);
		}
		fprintf(stderr, "	%.2f%%", (MC[i][i] / tmp) * 100);
	}

	for (i = 0; i < g2->nClasses + 1; i++)
		free(MC[i]);
	free(MC);

	fprintf(stdout, "\nComputando a Precisao...");
	fflush(stdout);
	float Acc2;
	Acc2 = opf_Precisao(g2);
	fprintf(stdout, "\nPrecisao: %.2f%%", Acc2 * 100);
	fflush(stdout);

	fprintf(stdout, "\nGravando a precisao no arquivo de saida ...");
	fflush(stdout);
	sprintf(fileName, "%s.acc", "./dataset_dat/teste.dat");
	f = fopen(fileName, "a");
	fprintf(f, "%f\n", Acc2 * 100);
	fclose(f);
	fprintf(stdout, " OK");
	fflush(stdout);

	fprintf(stdout, "\nDesalocando memoria ...");
	fflush(stdout);
	DestroiSubgrafico(&g2);
	fprintf(stdout, " OK\n");

    printf("\n\nfinal!");
	return 0;
}
