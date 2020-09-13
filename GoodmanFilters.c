/**
* File:   GoodmanFilters.c
* Applies a blur box or Swiss cheese filter to BMP images.
*
* Completion time: 12 hours
*
* @author Goodman, Acuna
* @version 2020.09.10
*/
////////////////////////////////////////////////////////////////////////////////
//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
//UNCOMMENT BELOW LINE IF USING SER334 LIBRARY/OBJECT FOR BMP SUPPORT
#include "BmpProcessor.h"

////////////////////////////////////////////////////////////////////////////////
//MACRO DEFINITIONS
#define THREAD_COUNT 4

////////////////////////////////////////////////////////////////////////////////
//DATA STRUCTURES
typedef struct Stencil {
    Pixel pixel[3][3];
}Stencil;

////////////////////////////////////////////////////////////////////////////////
//GLOBAL VARIABLES
int height, width;
Pixel **input_arr, **output_arr, *pixel;
Stencil* stencil;

////////////////////////////////////////////////////////////////////////////////
//FORWARD DECLARATIONS
void* blur_runner(void* param);
void blur_filter(Stencil* stencil, Pixel* pixel, int height, int width, Pixel** input_arr, Pixel** output_arr, int chunk);
Pixel* blur_pixel(Stencil* stencil, Pixel* pixel, int num_pixels);
void* cheese_runner(void* param);
void cheese_filter(int height, int width, Pixel** input_arr, Pixel** output_arr, int chunk);
void make_circle(int x_center, int y_center, int r, Pixel** pixel_array, int width, int height);
void draw_pixel(int x, int y, Pixel** pixel_array, int width, int height);

////////////////////////////////////////////////////////////////////////////////
//MAIN PROGRAM CODE
int main(int argc, char* argv[]){
    pthread_t tid;
    pthread_attr_t attr;
    int i, opt, length, arg_index = 0, i_flag = 0, o_flag = 0, f_flag = 0;
    char *input_file_name, *output_file_name, filter_type, **args;
    BMP_Header* input_bmp_header;
    DIB_Header* input_dib_header;
    BMP_Header* output_bmp_header;
    DIB_Header* output_dib_header;
    while((opt = getopt(argc, argv, "i:o:f:")) != -1)
        //parse command line arguments
        switch(opt){
            case 'i':
                if(arg_index != 0){
                    printf("First argument must be for input file. Exiting\n");
                    exit(1);
                }
                if(i_flag == 1){
                    printf("Extra argument for input file: %s. Additional argument discarded.\n", optarg);
                    arg_index++;
                    break;
                }
                arg_index++;
                i_flag = 1;
                input_file_name = optarg;
                break;
            case 'o':
                if(arg_index != 1){
                    printf("Second argument must be for output file name. Exiting\n");
                    exit(1);
                }
                if(o_flag == 1){
                    printf("Extra argument for output file name: %s.  Additional argument discarded.\n", optarg);
                    arg_index++;
                    break;
                }
                arg_index++;
                o_flag = 1;
                output_file_name = optarg;
                break;
            case 'f':
                if(arg_index != 2){
                    printf("Third argument must be filter type. Exiting\n");
                    exit(1);
                }
                if((optarg[0] != 'b' && optarg[0] != 'c') || strlen(optarg) != 1){
                    printf("Invalid filter type argument. Exiting\n\n");
                    exit(1);
                }
                if(f_flag == 1){
                    printf("Extra argument for filter type: %s.  Additional argument discarded.\n", optarg);
                    arg_index++;
                    break;
                }
                arg_index++;
                f_flag = 1;
                filter_type = optarg[0];
                break;
            case ':':
                printf("Option needs a value.\n");
                break;
            default:
                printf("Unknown option: %c.\n", optopt);
                break;
        }
    //verify input file is valid
    if(input_file_name != NULL){
        length = strlen(input_file_name);
        //read in bmp input file
        if(length >= 5
           && (strcmp(&input_file_name[length - 4], ".bmp") == 0)
           && (access(input_file_name, F_OK) != -1)){
            printf("Input: %s\n", input_file_name);
            FILE* input_file = fopen(input_file_name, "rb");
            //read a bmp header from file
            input_bmp_header = (BMP_Header*)malloc(sizeof(BMP_Header));
            readBMPHeader(input_file, input_bmp_header);
            //read a dib header from file
            input_dib_header = (DIB_Header*)malloc(sizeof(DIB_Header));
            readDIBHeader(input_file, input_dib_header);
            //read a bmp pixel array from file
            fseek(input_file, input_bmp_header->offset_pixel_array, SEEK_SET);
            input_arr = (Pixel**)malloc(input_dib_header->width * sizeof(Pixel*));
            for(i = 0; i < input_dib_header->width; i++){
                input_arr[i] = (Pixel*) malloc(input_dib_header->height * sizeof(Pixel));
            }
            output_arr = (Pixel**)malloc(input_dib_header->width * sizeof(Pixel*));
            for(i = 0; i < input_dib_header->width; i++){
                output_arr[i] = (Pixel*) malloc(input_dib_header->height * sizeof(Pixel));
            }
            readPixelsBMP(input_file, input_arr, input_dib_header->width, input_dib_header->height);
            fclose(input_file);
        }
        else {
            printf("Input file has an invalid name or is not accessible. Exiting.\n");
            exit(1);
        }
    }
    else {
        printf("No input file name provided. Exiting.\n");
        exit(1);
    }
    height = input_dib_header->height;
    width = input_dib_header->width;
    if(filter_type == 'b')
        //create blur box threads
        for(i = 0; i < THREAD_COUNT; i++) {
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, blur_runner, &i);
            pthread_join(tid, NULL);
        }
    else {
        //create Swiss cheese threads
        for (i = 0; i < THREAD_COUNT; i++) {
            pthread_attr_init(&attr);
            pthread_create(&tid, &attr, cheese_runner, &i);
            pthread_join(tid, NULL);
        }
        //determine smallest dimension of input
        int smallest = 0;
        if(width < height)
            smallest = width;
        else smallest = height;
        //calculate hole data
        int num_holes = smallest / 10;
        int average = num_holes;
        int large = average + average / 2;
        int small = average - average / 2;
        srand(time(0));
        //draw average holes (50% of holes)
        for(i = 0; i < num_holes / 2; i++) {
            int x_center = rand() % width;
            int y_center = rand() % height;
            int radius = average;
            make_circle(x_center, y_center, radius, output_arr, width, height);
        }
        //draw large holes (25% of holes)
        for(i = 0; i < num_holes / 4; i++) {
            int x_center = rand() % width;
            int y_center = rand() % height;
            int radius = large;
            make_circle(x_center, y_center, radius, output_arr, width, height);
        }
        //draw small holes (25% of holes)
        for(i = 0; i < num_holes / 4; i++) {
            int x_center = rand() % width;
            int y_center = rand() % height;
            int radius = small;
            make_circle(x_center, y_center, radius, output_arr, width, height);
        }
    }
    //produce an output file
    if(o_flag == 1){
        FILE* output_file = fopen(output_file_name, "wb");
        writeBMPHeader(output_file, input_bmp_header);
        writeDIBHeader(output_file, input_dib_header);
        writePixelsBMP(output_file, output_arr, input_dib_header->width, input_dib_header->height);
        fclose(output_file);
        for(i = 0; i < input_dib_header->width; i++)
            free(input_arr[i]);
        free(input_arr);
        for(i = 0; i < input_dib_header->width; i++)
            free(output_arr[i]);
        free(output_arr);
        free(input_bmp_header);
        free(input_dib_header);
        printf("Output: %s\n", output_file_name);
    }
    return 0;
}

void* blur_runner(void* param){
    int chunk_number = *(int*)param;
    blur_filter(stencil, pixel, height, width, input_arr, output_arr, chunk_number);
    printf("Thread %d complete.\n", chunk_number);
    pthread_exit(0);
}

void blur_filter(Stencil* stencil, Pixel* pixel, int height, int width, Pixel** input_arr, Pixel** output_arr, int chunk) {
    int i, j, k, left_offset, right_offset, left_padding = 0, right_padding = 0;
    int remaining = width % THREAD_COUNT;
    int chunk_width = width / THREAD_COUNT;
    left_offset = chunk_width * chunk;
    if(chunk == 0){
        right_offset = left_offset + chunk_width;
        chunk_width += 1;
        right_padding = 1;
    }
    else if(chunk > 0 && chunk < THREAD_COUNT - 1){
        right_offset = left_offset + chunk_width;
        chunk_width += + 2;
        left_padding = -1;
        right_padding = 1;
    }
    else{
        right_offset = left_offset + chunk_width + remaining;
        chunk_width += 1;
        left_padding = -1;
    }
    stencil = (Stencil*)malloc(sizeof(Stencil));
    pixel = (Pixel*)malloc(sizeof(Pixel));
    for(i = 0; i < height; i++){
        for(j = left_offset + left_padding; j < right_offset + right_padding; j++){
            //if pixel is upper left corner
            if(i == 0 && j == 0 && chunk == 0){
                for (k = 0; k < 3; k++) {
                    stencil->pixel[0][k].red = 0;
                    stencil->pixel[0][k].green = 0;
                    stencil->pixel[0][k].blue = 0;
                }
                for (k = 0; k < 3; k++) {
                    stencil->pixel[k][0].red = 0;
                    stencil->pixel[k][0].green = 0;
                    stencil->pixel[k][0].blue = 0;
                }
                stencil->pixel[1][1].red = input_arr[i][j].red;
                stencil->pixel[1][1].green = input_arr[i][j].green;
                stencil->pixel[1][1].blue = input_arr[i][j].blue;
                stencil->pixel[1][2].red = input_arr[i][j + 1].red;
                stencil->pixel[1][2].green = input_arr[i][j + 1].green;
                stencil->pixel[1][2].blue = input_arr[i][j + 1].blue;
                stencil->pixel[2][1].red = input_arr[i + 1][j].red;
                stencil->pixel[2][1].green = input_arr[i + 1][j].green;
                stencil->pixel[2][1].blue = input_arr[i + 1][j].blue;
                stencil->pixel[2][2].red = input_arr[i + 1][j + 1].red;
                stencil->pixel[2][2].green = input_arr[i + 1][j + 1].green;
                stencil->pixel[2][2].blue = input_arr[i + 1][j + 1].blue;
                output_arr[0][0] = *blur_pixel(stencil, pixel, 4);
            }
            //if pixel is upper right corner
            if(i == 0 && j == width - 1 && chunk == THREAD_COUNT - 1){
                for (k = 0; k < 3; k++) {
                    stencil->pixel[0][k].red = 0;
                    stencil->pixel[0][k].green = 0;
                    stencil->pixel[0][k].blue = 0;
                }
                for (k = 0; k < 3; k++) {
                    stencil->pixel[k][2].red = 0;
                    stencil->pixel[k][2].green = 0;
                    stencil->pixel[k][2].blue = 0;
                }
                stencil->pixel[1][0].red = input_arr[i][j - 1].red;
                stencil->pixel[1][0].green = input_arr[i][j - 1].green;
                stencil->pixel[1][0].blue = input_arr[i][j - 1].blue;
                stencil->pixel[1][1].red = input_arr[i][j].red;
                stencil->pixel[1][1].green = input_arr[i][j].green;
                stencil->pixel[1][1].blue = input_arr[i][j].blue;
                stencil->pixel[2][0].red = input_arr[i + 1][j - 1].red;
                stencil->pixel[2][0].green = input_arr[i + 1][j - 1].green;
                stencil->pixel[2][0].blue = input_arr[i + 1][j - 1].blue;
                stencil->pixel[2][1].red = input_arr[i + 1][j].red;
                stencil->pixel[2][1].green = input_arr[i + 1][j].green;
                stencil->pixel[2][1].blue = input_arr[i + 1][j].blue;
                output_arr[0][width - 1] = *blur_pixel(stencil, pixel, 4);
            }
            //if pixel is bottom left corner
            if(i == height - 1 && j == 0 && chunk == 0){
                for (k = 0; k < 3; k++) {
                    stencil->pixel[2][k].red = 0;
                    stencil->pixel[2][k].green = 0;
                    stencil->pixel[2][k].blue = 0;
                }
                for (k = 0; k < 3; k++) {
                    stencil->pixel[k][0].red = 0;
                    stencil->pixel[k][0].green = 0;
                    stencil->pixel[k][0].blue = 0;
                }
                stencil->pixel[0][1].red = input_arr[i - 1][j].red;
                stencil->pixel[0][1].green = input_arr[i - 1][j].green;
                stencil->pixel[0][1].blue = input_arr[i - 1][j].blue;
                stencil->pixel[0][2].red = input_arr[i - 1][j + 1].red;
                stencil->pixel[0][2].green = input_arr[i - 1][j + 1].green;
                stencil->pixel[0][2].blue = input_arr[i - 1][j + 1].blue;
                stencil->pixel[1][1].red = input_arr[i][j].red;
                stencil->pixel[1][1].green = input_arr[i][j].green;
                stencil->pixel[1][1].blue = input_arr[i][j].blue;
                stencil->pixel[1][2].red = input_arr[i][j + 1].red;
                stencil->pixel[1][2].green = input_arr[i][j + 1].green;
                stencil->pixel[1][2].blue = input_arr[i][j + 1].blue;
                output_arr[height - 1][0] = *blur_pixel(stencil, pixel, 4);
            }
            //if pixel is bottom right corner
            if(i == height - 1 && j == width - 1 && chunk == THREAD_COUNT - 1){
                for (k = 0; k < 3; k++) {
                    stencil->pixel[2][k].red = 0;
                    stencil->pixel[2][k].green = 0;
                    stencil->pixel[2][k].blue = 0;
                }
                for (k = 0; k < 3; k++) {
                    stencil->pixel[k][2].red = 0;
                    stencil->pixel[k][2].green = 0;
                    stencil->pixel[k][2].blue = 0;
                }
                stencil->pixel[0][0].red = input_arr[i - 1][j - 1].red;
                stencil->pixel[0][0].green = input_arr[i - 1][j - 1].green;
                stencil->pixel[0][0].blue = input_arr[i - 1][j - 1].blue;
                stencil->pixel[0][1].red = input_arr[i - 1][j].red;
                stencil->pixel[0][1].green = input_arr[i - 1][j].green;
                stencil->pixel[0][1].blue = input_arr[i - 1][j].blue;
                stencil->pixel[1][0].red = input_arr[i][j - 1].red;
                stencil->pixel[1][0].green = input_arr[i][j - 1].green;
                stencil->pixel[1][0].blue = input_arr[i][j - 1].blue;
                stencil->pixel[1][1].red = input_arr[i][j].red;
                stencil->pixel[1][1].green = input_arr[i][j].green;
                stencil->pixel[1][1].blue = input_arr[i][j].blue;
                output_arr[height - 1][width - 1] = *blur_pixel(stencil, pixel, 4);
            }
            //if pixel is on the left edge
            if(i != 0 && i != height - 1 && j == 0 && chunk == 0) {
                for (k = 0; k < 3; k++) {
                    stencil->pixel[k][0].red = 0;
                    stencil->pixel[k][0].green = 0;
                    stencil->pixel[k][0].blue = 0;
                }
                stencil->pixel[0][1].red = input_arr[i - 1][j].red;
                stencil->pixel[0][1].green = input_arr[i - 1][j].green;
                stencil->pixel[0][1].blue = input_arr[i - 1][j].blue;
                stencil->pixel[0][2].red = input_arr[i - 1][j + 1].red;
                stencil->pixel[0][2].green = input_arr[i - 1][j + 1].green;
                stencil->pixel[0][2].blue = input_arr[i - 1][j + 1].blue;
                stencil->pixel[1][1].red = input_arr[i][j].red;
                stencil->pixel[1][1].green = input_arr[i][j].green;
                stencil->pixel[1][1].blue = input_arr[i][j].blue;
                stencil->pixel[1][2].red = input_arr[i][j + 1].red;
                stencil->pixel[1][2].green = input_arr[i][j + 1].green;
                stencil->pixel[1][2].blue = input_arr[i][j + 1].blue;
                stencil->pixel[2][1].red = input_arr[i + 1][j].red;
                stencil->pixel[2][1].green = input_arr[i + 1][j].green;
                stencil->pixel[2][1].blue = input_arr[i + 1][j].blue;
                stencil->pixel[2][2].red = input_arr[i + 1][j + 1].red;
                stencil->pixel[2][2].green = input_arr[i + 1][j + 1].green;
                stencil->pixel[2][2].blue = input_arr[i + 1][j + 1].blue;
                output_arr[i][0] = *blur_pixel(stencil, pixel, 6);
            }
            //if pixel is on the right edge
            if(i != 0 && i != height - 1 && j == width - 1 && chunk == THREAD_COUNT - 1) {
                for (k = 0; k < 3; k++) {
                    stencil->pixel[k][2].red = 0;
                    stencil->pixel[k][2].green = 0;
                    stencil->pixel[k][2].blue = 0;
                }
                stencil->pixel[0][0].red = input_arr[i - 1][j - 1].red;
                stencil->pixel[0][0].green = input_arr[i - 1][j - 1].green;
                stencil->pixel[0][0].blue = input_arr[i - 1][j - 1].blue;
                stencil->pixel[0][1].red = input_arr[i - 1][j].red;
                stencil->pixel[0][1].green = input_arr[i - 1][j].green;
                stencil->pixel[0][1].blue = input_arr[i - 1][j].blue;
                stencil->pixel[1][0].red = input_arr[i][j - 1].red;
                stencil->pixel[1][0].green = input_arr[i][j - 1].green;
                stencil->pixel[1][0].blue = input_arr[i][j - 1].blue;
                stencil->pixel[1][1].red = input_arr[i][j].red;
                stencil->pixel[1][1].green = input_arr[i][j].green;
                stencil->pixel[1][1].blue = input_arr[i][j].blue;
                stencil->pixel[2][0].red = input_arr[i + 1][j - 1].red;
                stencil->pixel[2][0].green = input_arr[i + 1][j - 1].green;
                stencil->pixel[2][0].blue = input_arr[i + 1][j - 1].blue;
                stencil->pixel[2][1].red = input_arr[i + 1][j].red;
                stencil->pixel[2][1].green = input_arr[i + 1][j].green;
                stencil->pixel[2][1].blue = input_arr[i + 1][j].blue;
                output_arr[i][width - 1] = *blur_pixel(stencil, pixel, 6);
            }
            //if pixel is on the upper edge
            if(i == 0 && j != 0 && j != width - 1) {
                for (k = 0; k < 3; k++) {
                    stencil->pixel[0][k].red = 0;
                    stencil->pixel[0][k].green = 0;
                    stencil->pixel[0][k].blue = 0;
                }
                stencil->pixel[1][0].red = input_arr[i][j - 1].red;
                stencil->pixel[1][0].green = input_arr[i][j - 1].green;
                stencil->pixel[1][0].blue = input_arr[i][j - 1].blue;
                stencil->pixel[1][1].red = input_arr[i][j].red;
                stencil->pixel[1][1].green = input_arr[i][j].green;
                stencil->pixel[1][1].blue = input_arr[i][j].blue;
                stencil->pixel[1][2].red = input_arr[i][j + 1].red;
                stencil->pixel[1][2].green = input_arr[i][j + 1].green;
                stencil->pixel[1][2].blue = input_arr[i][j + 1].blue;
                stencil->pixel[2][0].red = input_arr[i + 1][j - 1].red;
                stencil->pixel[2][0].green = input_arr[i + 1][j - 1].green;
                stencil->pixel[2][0].blue = input_arr[i + 1][j - 1].blue;
                stencil->pixel[2][1].red = input_arr[i + 1][j].red;
                stencil->pixel[2][1].green = input_arr[i + 1][j].green;
                stencil->pixel[2][1].blue = input_arr[i + 1][j].blue;
                stencil->pixel[2][2].red = input_arr[i + 1][j + 1].red;
                stencil->pixel[2][2].green = input_arr[i + 1][j + 1].green;
                stencil->pixel[2][2].blue = input_arr[i + 1][j + 1].blue;
                output_arr[0][j] = *blur_pixel(stencil, pixel, 6);
            }
            //if pixel is on the bottom edge
            if(i == height - 1 && j != 0 && j != width - 1) {
                for (k = 0; k < 3; k++) {
                    stencil->pixel[2][k].red = 0;
                    stencil->pixel[2][k].green = 0;
                    stencil->pixel[2][k].blue = 0;
                }
                stencil->pixel[0][0].red = input_arr[i - 1][j - 1].red;
                stencil->pixel[0][0].green = input_arr[i - 1][j - 1].green;
                stencil->pixel[0][0].blue = input_arr[i - 1][j - 1].blue;
                stencil->pixel[0][1].red = input_arr[i - 1][j].red;
                stencil->pixel[0][1].green = input_arr[i - 1][j].green;
                stencil->pixel[0][1].blue = input_arr[i - 1][j].blue;
                stencil->pixel[0][2].red = input_arr[i - 1][j + 1].red;
                stencil->pixel[0][2].green = input_arr[i - 1][j + 1].green;
                stencil->pixel[0][2].blue = input_arr[i - 1][j + 1].blue;
                stencil->pixel[1][0].red = input_arr[i][j - 1].red;
                stencil->pixel[1][0].green = input_arr[i][j - 1].green;
                stencil->pixel[1][0].blue = input_arr[i][j - 1].blue;
                stencil->pixel[1][1].red = input_arr[i][j].red;
                stencil->pixel[1][1].green = input_arr[i][j].green;
                stencil->pixel[1][1].blue = input_arr[i][j].blue;
                stencil->pixel[1][2].red = input_arr[i][j + 1].red;
                stencil->pixel[1][2].green = input_arr[i][j + 1].green;
                stencil->pixel[1][2].blue = input_arr[i][j + 1].blue;
                output_arr[height - 1][j] = *blur_pixel(stencil, pixel, 6);
            }
            //if pixel is not on the border
            if(i != 0 && i != height - 1 && j != 0 && j != width - 1) {
                stencil->pixel[0][0].red = input_arr[i - 1][j - 1].red;
                stencil->pixel[0][0].green = input_arr[i - 1][j - 1].green;
                stencil->pixel[0][0].blue = input_arr[i - 1][j - 1].blue;
                stencil->pixel[0][1].red = input_arr[i - 1][j].red;
                stencil->pixel[0][1].green = input_arr[i - 1][j].green;
                stencil->pixel[0][1].blue = input_arr[i - 1][j].blue;
                stencil->pixel[0][2].red = input_arr[i - 1][j + 1].red;
                stencil->pixel[0][2].green = input_arr[i - 1][j + 1].green;
                stencil->pixel[0][2].blue = input_arr[i - 1][j + 1].blue;
                stencil->pixel[1][0].red = input_arr[i][j - 1].red;
                stencil->pixel[1][0].green = input_arr[i][j - 1].green;
                stencil->pixel[1][0].blue = input_arr[i][j - 1].blue;
                stencil->pixel[1][1].red = input_arr[i][j].red;
                stencil->pixel[1][1].green = input_arr[i][j].green;
                stencil->pixel[1][1].blue = input_arr[i][j].blue;
                stencil->pixel[1][2].red = input_arr[i][j + 1].red;
                stencil->pixel[1][2].green = input_arr[i][j + 1].green;
                stencil->pixel[1][2].blue = input_arr[i][j + 1].blue;
                stencil->pixel[2][0].red = input_arr[i + 1][j - 1].red;
                stencil->pixel[2][0].green = input_arr[i + 1][j - 1].green;
                stencil->pixel[2][0].blue = input_arr[i + 1][j - 1].blue;
                stencil->pixel[2][1].red = input_arr[i + 1][j].red;
                stencil->pixel[2][1].green = input_arr[i + 1][j].green;
                stencil->pixel[2][1].blue = input_arr[i + 1][j].blue;
                stencil->pixel[2][2].red = input_arr[i + 1][j + 1].red;
                stencil->pixel[2][2].green = input_arr[i + 1][j + 1].green;
                stencil->pixel[2][2].blue = input_arr[i + 1][j + 1].blue;
                output_arr[i][j] = *blur_pixel(stencil, pixel, 9);
            }
        }
    }
    free(pixel);
    free(stencil);
}

Pixel* blur_pixel(Stencil* stencil, Pixel* pixel, int num_pixels) {
    int i, j, red_sum = 0, green_sum = 0, blue_sum = 0;
    for(i = 0; i < 3; i++)
        for(j = 0; j < 3; j++){
            red_sum += stencil->pixel[i][j].red;
            green_sum += stencil->pixel[i][j].green;
            blue_sum += stencil->pixel[i][j].blue;
        }
    pixel->red = red_sum / num_pixels;
    pixel->green = green_sum / num_pixels;
    pixel->blue = blue_sum / num_pixels;
    return pixel;
}

void* cheese_runner(void* param){
    int chunk_number = *(int*)param;
    cheese_filter(height, width, input_arr, output_arr, chunk_number);
    printf("Thread %d complete.\n", chunk_number);
    pthread_exit(0);
}

void cheese_filter(int height, int width, Pixel** input_arr, Pixel** output_arr, int chunk) {
    int i, j, left_offset, right_offset;
    int remaining = width % THREAD_COUNT;
    int chunk_width = width / THREAD_COUNT;
    left_offset = chunk_width * chunk;
    if(chunk != THREAD_COUNT - 1)
        right_offset = left_offset + chunk_width;
    else
        right_offset = left_offset + chunk_width + remaining;
    //apply yellow tint
    for(i = 0; i < height; i++)
        for(j = left_offset; j < right_offset; j++){
            if(input_arr[i][j].red + 50 > 255)
                output_arr[i][j].red = 255;
            else output_arr[i][j].red = input_arr[i][j].red + 50;
            if(input_arr[i][j].green + 50 > 255)
                output_arr[i][j].green = 255;
            else output_arr[i][j].green = input_arr[i][j].green + 50;
            output_arr[i][j].blue = input_arr[i][j].blue;
        }
}

void make_circle(int x_center, int y_center, int r, Pixel** pixel_array, int width, int height) {
    int y, x;
    for (y = -r; y <= r; y++)
        for (x = -r; x <= r; x++)
            if (x * x + y * y <= r * r)
                draw_pixel(x_center + x, y_center + y, pixel_array, width, height);
}

void draw_pixel(int x, int y, Pixel** pixel_array, int width, int height){
    if(x >= 0 && x < width && y >= 0 && y < height){
        pixel_array[x][y].red = 0;
        pixel_array[x][y].green = 0;
        pixel_array[x][y].blue = 0;
    }
}