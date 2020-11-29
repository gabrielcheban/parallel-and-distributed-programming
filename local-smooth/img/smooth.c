
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


// #define WIDTH  7680
#define WIDTH  512
// #define HEIGHT 4320
#define HEIGHT 512

int mr [WIDTH][HEIGHT];
int mg [WIDTH][HEIGHT];
int mb [WIDTH][HEIGHT];
int ma [WIDTH][HEIGHT];
int mr2 [WIDTH][HEIGHT];
int mg2 [WIDTH][HEIGHT];
int mb2 [WIDTH][HEIGHT];
int ma2 [WIDTH][HEIGHT];

int 
main(int argc, char **argv)
{
	int i,j;
	int fdi, fdo;
	int nlin = 0;
	int ncol = 0;
	char name[128];

	if(argc<2) {
		printf("Uso: %s nome_arquivo\n",argv[0]);
		exit(0);
	}
	if((fdi=open(argv[1],O_RDONLY))==-1) {
		printf("Erro na abertura do arquivo %s\n",argv[1]);
		exit(0);
	}

	// lê arquivo de imagem
	// formato: 
	//		ncolunas (2bytes) - largura
	//		nlinhas (2bytes) - altura
	// dados: rgba (4bytes)

/*
 * Supondo imagem 512x512, do exemplo, não mantemos tamanho no arquivo
 *
	if(read(fdi,&ncol,2)==-1) {
		printf("Erro na leitura do arquivo\n");		
		exit(0);
	}
	if(read(fdi,&nlin,2)==-1) {
		printf("Erro na leitura do arquivo\n");		
		exit(0);
	}
	printf("Tamanho da imagem: %d x %d\n",ncol,nlin);
*/
	nlin=ncol=512;
	
	// zerar as matrizes (4 bytes, mas usaremos 1 por pixel)
	// void *memset(void *s, int c, size_t n);
	memset(mr,0,nlin*ncol*sizeof(int));
	memset(mg,0,nlin*ncol*sizeof(int));
	memset(mb,0,nlin*ncol*sizeof(int));
	memset(ma,0,nlin*ncol*sizeof(int));
	memset(mr2,0,nlin*ncol*sizeof(int));
	memset(mg2,0,nlin*ncol*sizeof(int));
	memset(mb2,0,nlin*ncol*sizeof(int));
	memset(ma2,0,nlin*ncol*sizeof(int));

	// (ao menos) 2 abordagens: 
	// - ler pixels byte a byte, colocando-os em matrizes separadas
	//	- ler pixels (32bits) e depois usar máscaras e rotações de bits para o processamento.

	// ordem de leitura dos bytes (componentes do pixel) depende se o formato
	// é little ou big endian
	// Assumindo little endian
	for(i=0;i<nlin;i++) {
		for(j=0;j<ncol;j++) {
			read(fdi,&mr[i][j],1);
			read(fdi,&mg[i][j],1);
			read(fdi,&mb[i][j],1);
			read(fdi,&ma[i][j],1);
		}
	}
	close(fdi);

	// aplicar filtro (estêncil)
	// repetir para mr2, mg2, mb2, ma2

	// tratar: linhas 0, 1, n, n-1; colunas 0,1,n,n-1
	// for

	// gravar imagem resultante
	sprintf(name,"%s.new",argv[1]);	
	fdo=open(name,O_WRONLY|O_CREAT);

/*
	write(fdo,&ncol,2);
	write(fdo,&nlin,2);
*/

	for(i=0;i<nlin;i++) {
		for(j=0;j<ncol;j++) {
			write(fdo,&mr2[i][j],1);
			write(fdo,&mg2[i][j],1);
			write(fdo,&mb2[i][j],1);
			write(fdo,&ma2[i][j],1);
		}
	}
	close(fdo);
	
	return 0;
}


