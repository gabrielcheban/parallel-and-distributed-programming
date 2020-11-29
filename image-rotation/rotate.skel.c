/* 
 ** Universidade Federal de São Carlos
 ** Departamento de Computação
 ** Prof. Hélio Crestana Guardia
 ** Programação Paralela e Distribuída
 */

/*
 ** Programa : 
 ** Comando: 
 ** Objetivo:
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h> 
#include <sys/time.h>
#include <sys/resource.h>


#define WIDTH  512
#define HEIGHT 512


unsigned int img_orig [HEIGHT][WIDTH];
unsigned int img_rot [HEIGHT][WIDTH];

int 
main(int argc, char **argv)
{
	int i,j,x,y,xc,yc;
	int fd;
	int nlin = 0;
	int ncol = 0;
	char fname[128];
	float angle = 90;
	float s,c;

	// variáveis para medida do tempo
	struct timeval inic,fim;
	struct rusage r1, r2;

	if(argc<3) {
		printf("Uso: %s nome_arquivo angulo  // (sem extensão .rgba)\n",argv[0]);
		exit(0);
	}

	sprintf(fname,"%s.rgba",argv[1]);	
	if((fd=open(fname,O_RDONLY))==-1) {
		printf("Erro na abertura do arquivo %s\n",argv[1]);
		exit(0);
	}
   angle = atof(argv[2]);
   if(angle<=0 || angle>360) {
     	printf("Erro: angulo deve estar entre 1 e 360\n");
     	exit(0);
   }   

	// Lê arquivo de imagem. Formato: 
	//		ncolunas (2bytes) - largura
	//		nlinhas (2bytes) - altura
	// dados: rgba (4bytes)

/*
 * Supondo imagem 512x512, do exemplo, não mantemos tamanho no arquivo
 *
	if(read(fd,&ncol,2)==-1) {
		printf("Erro na leitura do arquivo\n");		
		exit(0);
	}
	if(read(fd,&nlin,2)==-1) {
		printf("Erro na leitura do arquivo\n");		
		exit(0);
	}
	printf("Tamanho da imagem: %d x %d\n",ncol,nlin);
*/
	nlin=HEIGHT;
	ncol=WIDTH;

	// trabalhar com 2 imagens, a original e a transformada...

	// Precisa zerar as matrizes?
	// void *memset(void *s, int c, size_t n);
	// memset(img_orig,0,WIDTH*HEIGHT*sizeof(int));
	memset(img_rot,0,WIDTH*HEIGHT*sizeof(int));

	// ordem dos bytes (componentes do pixel) depende se formato é little ou big endian
	// Assumindo little endian
	for(i=0;i<nlin;i++) 
		for(j=0;j<ncol;j++) 
			read(fd,&img_orig[i][j],4);
	// leitura em bloco (nlin * ncol * 4) pode gerar melhor desempenho?
	// dividir em blocos de 4k? (tamanho do bloco do sistema de arquivos)
	
	// fecha arquivo lido
	close(fd);

	// obtém tempo e consumo de CPU antes da aplicação do filtro
	gettimeofday(&inic,0);
	getrusage(RUSAGE_SELF, &r1);

	// aplicar transformação

	// Rotação em torno de um ponto
	// x' = (x-x0) cosΘ - (y-y0) sinΘ + x0;
	// y' = (x-x0) sinΘ + (y-y0) cosΘ + y0;

	// Considerando rotação em torno do ponto central
	xc = ncol/2;
	yc = nlin/2;

	// converting angle to radians
	angle = angle * M_PI / 180.0;
	s = sin(angle);
	c = cos(angle);

	for(i=0;i<nlin;i++)
		for(j=0;j<ncol;j++) {

			// faz a rotação dos pixels no sentido horario, em torno do ponto central...

			x = (j-xc)*c -(i-yc)*s + xc;
			y = (j-xc)*s +(i-yc)*c + yc;

			// verifica se o ponto está dentro da área da imagem antes de atribuir `a matriz
         if(x >= 0 && x < ncol && y >= 0 && y < nlin)
            img_rot[y][x] = img_orig[i][j];
		}

	// obtém tempo e consumo de CPU depois da aplicação do filtro
	gettimeofday(&fim,0);
	getrusage(RUSAGE_SELF, &r2);

	printf("\nElapsed time:%f sec\tUser time:%f sec\tSystem time:%f sec\n",
	 (fim.tv_sec+fim.tv_usec/1000000.) - (inic.tv_sec+inic.tv_usec/1000000.),
	 (r2.ru_utime.tv_sec+r2.ru_utime.tv_usec/1000000.) - (r1.ru_utime.tv_sec+r1.ru_utime.tv_usec/1000000.),
	 (r2.ru_stime.tv_sec+r2.ru_stime.tv_usec/1000000.) - (r1.ru_stime.tv_sec+r1.ru_stime.tv_usec/1000000.));


	// gravar imagem resultante
	sprintf(fname,"%s.%s.rgba",argv[1],argv[2]);	
	fd=open(fname,O_WRONLY|O_CREAT,0644);

	// write(fd,&ncol,2); write(fd,&nlin,2);
	for(i=0;i<nlin;i++) 
		for(j=0;j<ncol;j++) 
			write(fd,&img_rot[i][j],4);
	
	close(fd);
	
	return 0;
}



