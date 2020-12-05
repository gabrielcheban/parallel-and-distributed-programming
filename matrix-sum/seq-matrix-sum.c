#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  FILE *arquivoA = NULL;
  FILE *arquivoB = NULL;
  FILE *arquivoR = NULL;

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

  for(int i = 0; i < linhas; i++) {
    // Somando duas linhas
    for(int j = 0; j < colunas; j++)
      matrizR[i][j] = matrizA[i][j] + matrizB[i][j];
  }

  arquivoR = fopen("soma.txt", "w");
  // Salva os valores da matriz resultante em um arquivo
  for(int i=0; i<linhas; i++){
    for(int j=0; j<colunas; j++)
      fprintf(arquivoR,"%.2f ", matrizR[i][j]);
    fprintf(arquivoR,"\n");
  }

  fclose(arquivoR);
  arquivoR = NULL;

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
