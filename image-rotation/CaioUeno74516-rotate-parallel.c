/* 
 ** Universidade Federal de São Carlos
 ** Departamento de Computação
 ** Prof. Hélio Crestana Guardia
 ** Programação Paralela e Distribuída

 ** Aluno Caio Ueno 743516 Bacharelado em Ciencia da Computacao
 */

/*
 ** Objetivo: Rotacionar uma imagem *.rgba de um angulo fornecido pelo usuario utilizando paralelismo.
              Baseado no programa rotate.skel.c - sequencial.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>          // usar flag -lm (link compiler)
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>       // usar flag -pthread (link compiler)

#define WIDTH  512
#define HEIGHT 512
#define NTHREADS 8

// variaveis globais
int xc, yc;
float s, c;

// imagens
unsigned int img_orig [HEIGHT][WIDTH];
unsigned int img_rot [HEIGHT][WIDTH];

// pthreads
pthread_t threads[NTHREADS];

//thread que aplica a rotacao
void *rot_col_img(void *arg) {

    int x, y;
    int* offset = (int *) arg;
    
    // realiza a rotacao somente nas linhas do offset dado
    for(int i=*offset; i < HEIGHT; i += NTHREADS) 
        for(int j=0; j < WIDTH; j++) {

            x = (j-xc)*c -(i-yc)*s + xc;
            y = (j-xc)*s +(i-yc)*c + yc;

            if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
            img_rot[y][x] = img_orig[i][j];
        }

}

int main(int argc, char **argv)
{

    // checagens de entrada
    if(argc < 3) {
        printf("Uso: %s nome_arquivo angulo  // (sem extensão .rgba)\n", argv[0]);
        exit(0);
    }

    int fd;
    char fname[128];
    float angle = 0;

    sprintf(fname, "%s.rgba", argv[1]);	

    if((fd=open(fname,O_RDONLY))==-1) {
        printf("Erro na abertura do arquivo %s\n", argv[1]);
        exit(0);
    }

    angle = atof(argv[2]);

    if(angle<=0 || angle>360) {
        printf("Erro: angulo deve estar entre 1 e 360\n");
        exit(0);
    }   

    // inicializando imagem rotacionada
    memset(img_rot, 0, WIDTH * HEIGHT * sizeof(int));

    // ordem dos bytes (componentes do pixel) depende se formato é little ou big endian
    // Assumindo little endian
    for(int i=0; i < HEIGHT; i++) 
        for(int j=0; j < WIDTH; j++) 
            read(fd, &img_orig[i][j], 4);

    close(fd);

    // variáveis para medida do tempo
	struct timeval inic,fim;
	struct rusage r1, r2;

    // obtém tempo e consumo de CPU antes da aplicação do filtro
	gettimeofday(&inic,0);
	getrusage(RUSAGE_SELF, &r1);

    // Considerando rotação em torno do ponto central
	xc = WIDTH/2;
	yc = HEIGHT/2;

	// angulo em radianos
	angle = angle * M_PI / 180.0;
	s = sin(angle);
	c = cos(angle);

    int r;
    int* offsets = (int *) malloc(NTHREADS * sizeof(int)); // vetor com os possíveis offsets 
                                                           // usar o contador (n_thread) como parametro da thread causará erro,
                                                           // pois quando a thread começar, o valor já pode ter sido incrementado para o 
                                                           // próximo loop

    for (int n_thread = 0; n_thread < NTHREADS; n_thread++) {
        
        offsets[n_thread] = n_thread;
        r = pthread_create(&threads[n_thread], NULL, &rot_col_img, &offsets[n_thread]);
        
        if (r != 0) {
            printf("Erro para criar thread\n");
            exit(0);
        }
    }
    
    // join de todas as threads
    for(int n_thread = 0; n_thread < NTHREADS; n_thread++)
        pthread_join(threads[n_thread], NULL);

    // obtém tempo e consumo de CPU depois da aplicação do filtro
	gettimeofday(&fim, 0);
	getrusage(RUSAGE_SELF, &r2);

	printf("\nElapsed time:%f sec\tUser time:%f sec\tSystem time:%f sec\n",
	 (fim.tv_sec+fim.tv_usec/1000000.) - (inic.tv_sec+inic.tv_usec/1000000.),
	 (r2.ru_utime.tv_sec+r2.ru_utime.tv_usec/1000000.) - (r1.ru_utime.tv_sec+r1.ru_utime.tv_usec/1000000.),
	 (r2.ru_stime.tv_sec+r2.ru_stime.tv_usec/1000000.) - (r1.ru_stime.tv_sec+r1.ru_stime.tv_usec/1000000.));

    
    // gravar imagem resultante
	sprintf(fname, "%s.%s.rgba", argv[1], argv[2]);	
	fd=open(fname, O_WRONLY|O_CREAT, 0644);

	for(int i = 0; i < HEIGHT; i++) 
		for(int j = 0; j < WIDTH; j++) 
			write(fd, &img_rot[i][j], 4);
	
	close(fd);

    return 0;
}

