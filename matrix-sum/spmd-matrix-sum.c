#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mpi.h"

int main(int argc, char **argv) {

  int linhas, colunas;

  // Verifica se os argumentos foram fornecidos
  if (argc < 5) {
      printf("Uso do programa: ./programa matrizA.txt matrizB.txt n_linhas n_colunas\n");
      exit(0);
  }

  linhas = atof(argv[3]);
  colunas = atof(argv[4]);

  // Declaração das variáveis
  float **matrizA, **matrizB, **matrizR;
  float *message;
  int linha_offset;
  int rank, size, qtd_processos, type;
  MPI_Status status;
  FILE *arquivoA = NULL;
  FILE *arquivoB = NULL;
  FILE *arquivoR = NULL;

  // variáveis para medida do tempo
	struct timeval inic,fim;
	struct rusage r1, r2;

  message = (float *) malloc(colunas*sizeof(float *));

  // Alocação dinâmica das matrizes
  matrizA = (float **) malloc(linhas*sizeof(float *));
  matrizB = (float **) malloc(linhas*sizeof(float *));
  matrizR = (float **) malloc(linhas*sizeof(float *));

  for(int i=0; i<linhas; i++){
    matrizA[i] = (float *) malloc(colunas*sizeof(float));
    matrizB[i] = (float *) malloc(colunas*sizeof(float));
    matrizR[i] = (float *) malloc(colunas*sizeof(float));
  }

  // Abertura dos arquivos
  arquivoA = fopen(argv[1], "r");
  arquivoB = fopen(argv[2], "r");

  // Leitura dos arquivos
  for(int i=0; i<linhas; i++){
    for(int j=0; j<colunas; j++){
      fscanf(arquivoA,"%f ", &matrizA[i][j]);
      fscanf(arquivoB,"%f ", &matrizB[i][j]);
    }
  }

  // Fecha os arquivos
  fclose(arquivoA);
  fclose(arquivoB);
  arquivoA = NULL;
  arquivoB = NULL;

  // obtém tempo e consumo de CPU antes da aplicação do filtro
	gettimeofday(&inic,0);
	getrusage(RUSAGE_SELF, &r1);

  // Inicializa MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  // Trecho executado apenas pelo mestre
  if (rank == 0){

    for(int i = 0; i < linhas; i++){

      // Recebe o resultado da linha i do processo [i%(size-1)+1]
      MPI_Recv(&message[0], colunas, MPI_FLOAT, i%(size-1)+1, 99, MPI_COMM_WORLD, &status);

      for(int j = 0; j < colunas; j++)
        matrizR[i][j] = message[j];

    }
      // obtém tempo e consumo de CPU depois da aplicação do filtro
    gettimeofday(&fim,0);
    getrusage(RUSAGE_SELF, &r2);

    printf("\nElapsed time:%f sec\tUser time:%f sec\tSystem time:%f sec\n",
    (fim.tv_sec+fim.tv_usec/1000000.) - (inic.tv_sec+inic.tv_usec/1000000.),
    (r2.ru_utime.tv_sec+r2.ru_utime.tv_usec/1000000.) - (r1.ru_utime.tv_sec+r1.ru_utime.tv_usec/1000000.),
    (r2.ru_stime.tv_sec+r2.ru_stime.tv_usec/1000000.) - (r1.ru_stime.tv_sec+r1.ru_stime.tv_usec/1000000.));
    
    // Salva os valores da matriz resultante em um arquivo
    arquivoR = fopen("soma.txt", "w");

    for(int i=0; i<linhas; i++){
      for(int j=0; j<colunas; j++)
        fprintf(arquivoR,"%.2f ", matrizR[i][j]);
      fprintf(arquivoR,"\n");
    }

    fclose(arquivoR);
    arquivoR = NULL;
  }

  // Trecho executado apenas pelos trabalhadores
  else {
    for(int i = rank-1; i < linhas; i = i+size-1) {

      // Somando duas linhas
      for(int j = 0; j < colunas; j++)
        message[j] = matrizA[i][j] + matrizB[i][j];

      // Manda o resultado da soma para o processo mestre
      MPI_Send(&message[0], colunas, MPI_FLOAT, 0, 99, MPI_COMM_WORLD);
    }
  }

  MPI_Finalize();

  // Desaloca as matrizes
  for(int i=0; i<linhas; i++){
    free(matrizA[i]);
    free(matrizB[i]);
    free(matrizR[i]);
    matrizA[i] = NULL;
    matrizB[i] = NULL;
    matrizR[i] = NULL;
  }

  free(matrizA);
  free(matrizB);
  free(matrizR);
  matrizA = NULL;
  matrizB = NULL;
  matrizR = NULL;

  return 0;
}
