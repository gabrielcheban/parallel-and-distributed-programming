#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <omp.h>

#define NTHREADS 4

// global variables for width and height
int WIDTH, HEIGHT;

// input image
unsigned int** INPUT_IMG;
unsigned int** SMOOTHED_IMG;

unsigned int select_channel(unsigned int pixel, int channel_offset){

    /**
     * Given a pixel (4 bytes) and a channel (RGB) it returns 
     * the value of the selected channel using shift operations.
    **/

    switch (channel_offset) {
    
    case 0: // red channel
        return pixel >> 8 * 3; // right shift 3 bytes

    case 1: // green channel
        return (pixel << 8) >> 8 * 3; // left shift 1 byte (remove red value) and right shift 3 bytes 

    case 2: // blue channel
        return  (pixel << 8 * 2) >> 8 * 3; // left shift 2 bytes (remove red and green) and right shift 3 bytes 

    default:
        break;
    }
}

unsigned int reallocated_channel(unsigned int value, int channel_offset){
    
    /**
     * It reallocate the value to the selected channel position.
    **/

    switch (channel_offset) {
    
    case 0: // red channel
        return value << 8 * 3; // left shift 3 bytes

    case 1: // green channel
        return value << 8 * 2; // left shift 2 bytes

    case 2: // blue channel
        return value << 8; // left shift 1 byte

    default:
        break;
    }

}

int smooth_filter(int pivot_pixel_row, int pivot_pixel_column, int channel_offset) {

    /**
     * Main function to apply the smoothing. It get every pixel around
     * the pivot pixel (neighborhood) and calculate the mean for a selected channel.
    **/

    //      pixels location reference
    //      +--------------+
    //      | x0 | x1 | x2 |
    //      |----|----|----|
    //      | x3 | x4 | x5 |
    //      |----|----|----|
    //      | x6 | x7 | x8 |
    //      +--------------+
    
    unsigned int pixels_map[9];

    // filling neighborhood
    // first row
    pixels_map[0] = INPUT_IMG[pivot_pixel_row-1][pivot_pixel_column-1];
    pixels_map[1] = INPUT_IMG[pivot_pixel_row-1][pivot_pixel_column];
    pixels_map[2] = INPUT_IMG[pivot_pixel_row-1][pivot_pixel_column+1];
    
    // second row
    pixels_map[3] = INPUT_IMG[pivot_pixel_row][pivot_pixel_column-1];
    pixels_map[4] = INPUT_IMG[pivot_pixel_row][pivot_pixel_column];
    pixels_map[5] = INPUT_IMG[pivot_pixel_row][pivot_pixel_column+1];
    
    // third row
    pixels_map[6] = INPUT_IMG[pivot_pixel_row+1][pivot_pixel_column-1];
    pixels_map[7] = INPUT_IMG[pivot_pixel_row+1][pivot_pixel_column];
    pixels_map[8] = INPUT_IMG[pivot_pixel_row+1][pivot_pixel_column+1];
    
    // selecting channel
    for (int i = 0; i < 9; i++)
        pixels_map[i] = select_channel(pixels_map[i], channel_offset);
    
    // mean of neighborhood pixels
    unsigned int sum = 0;
    unsigned int mean;
    for (int p = 0; p < 9; p++)
        sum += pixels_map[p];
    
    mean = sum / 9;

    // put the value on the right channel
    mean = reallocated_channel(mean, channel_offset);
    
    return mean;
}

int main(int argc, char const *argv[]) {

    // checking input arguments
    if(argc < 4) {
		printf("Uso: %s nome_arquivo largura_do_arquivo altura_do_arquivo\n", argv[0]);
		exit(0);
	}

    // setting image dimensions
    WIDTH = atof(argv[2]);
    HEIGHT = atof(argv[3]);

    // file "descriptors"
    int fdi, fdo;

    // open file
	if((fdi = open(argv[1], O_RDONLY)) == -1) {
		printf("Erro na abertura do arquivo %s\n", argv[1]);
		exit(0);
	}

    // dynamic allocation for images - calloc initialize arrays with zeros.
    // INPUT IMG is padded using zeros adding 2 rows and 2 coluns to its true dimensions
    // considering a 3x3 neighborhood.
    // A image is a matrix using unsigned int to store all four values RGBA.

    INPUT_IMG = (unsigned int **) calloc(HEIGHT + 2, sizeof(unsigned int *));
    for (int i = 0; i < HEIGHT + 2; i++)
        INPUT_IMG[i] = (unsigned int *) calloc(WIDTH + 2, sizeof(unsigned int));
    
    // SMOOTHED IMG is not padded
    SMOOTHED_IMG = (unsigned int **) calloc(HEIGHT, sizeof(unsigned int *));
    for (int i = 0; i < HEIGHT; i++)
        SMOOTHED_IMG[i] = (unsigned int *) calloc(WIDTH, sizeof(unsigned int));
    
    // reading input image
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            read(fdi, &INPUT_IMG[i][j], 4);
    
    // channel offsets
    int RED_OFFSET = 0, GREEN_OFFSET = 1, BLUE_OFFSET = 2;

    // using OpenMP
    #pragma omp parallel for
    for (int i = 0; i < HEIGHT; i++) {    

        for (int j = 0; j < WIDTH; j++) {
            
            // one variable for each channel
            unsigned int red_pixel, green_pixel, blue_pixel, alpha;
            red_pixel = smooth_filter(i+1, j+1, RED_OFFSET);
            green_pixel = smooth_filter(i+1, j+1, GREEN_OFFSET);
            blue_pixel = smooth_filter(i+1, j+1, BLUE_OFFSET);
            alpha = (INPUT_IMG[i+1][j+1] << 8 * 3) >> 8 * 3; // removing only RGB values

            SMOOTHED_IMG[i][j] = red_pixel | green_pixel | blue_pixel | alpha;
        }
        
    }
    
    char name[128];

    // smoothed image
	sprintf(name, "smoothed-%s", argv[1]);	
	fdo = open(name, O_WRONLY|O_CREAT);

    // writing smoothed image
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            write(fdo, &SMOOTHED_IMG[i][j], 4);
            
	close(fdo);

    return 0;
}
