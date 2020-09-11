/**
* File:   GoodmanFilters.c
* Applies a blur box or Swiss cheese filter to BMP images.
*
* Completion time: (estimation of hours spent on this program)
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
#include <math.h>
#include <unistd.h>
//UNCOMMENT BELOW LINE IF USING SER334 LIBRARY/OBJECT FOR BMP SUPPORT
#include "BmpProcessor.h"

////////////////////////////////////////////////////////////////////////////////
//MACRO DEFINITIONS
#define BMP_HEADER_SIZE 14
#define BMP_DIB_HEADER_SIZE 40
#define MAXIMUM_IMAGE_SIZE 4096
#define THREAD_COUNT 4

////////////////////////////////////////////////////////////////////////////////
//DATA STRUCTURES
typedef struct Stencil {
    Pixel pixel[3][3];
}Stencil;

////////////////////////////////////////////////////////////////////////////////
//FORWARD DECLARATIONS
Pixel* make_blurry_pixel(Stencil* stencil, Pixel* pixel, int numPixels);

////////////////////////////////////////////////////////////////////////////////
//MAIN PROGRAM CODE
int main(int argc, char* argv[]){
    int i, j, k, opt, length, argIndex = 0, iFlag = 0, oFlag = 0, fFlag = 0;
    char *input_file, *output_file, filter_type;
    BMP_Header* input_bmp_header;
    DIB_Header* input_dib_header;
    BMP_Header* output_bmp_header;
    DIB_Header* output_dib_header;
    Pixel** inputArr;
    while((opt = getopt(argc, argv, "i:o:f:")) != -1)
        //parse command line arguments
        switch(opt){
            case 'i':
                if(argIndex != 0){
                    printf("First argument must be for input file. Exiting\n");
                    exit(1);
                }
                if(iFlag == 1){
                    printf("Extra argument for input file: %s. Additional argument discarded.\n", optarg);
                    argIndex++;
                    break;
                }
                argIndex++;
                iFlag = 1;
                input_file = optarg;
                break;
            case 'o':
                if(argIndex != 1){
                    printf("Second argument must be for output file name. Exiting\n");
                    exit(1);
                }
                if(oFlag == 1){
                    printf("Extra argument for output file name: %s.  Additional argument discarded.\n", optarg);
                    argIndex++;
                    break;
                }
                argIndex++;
                oFlag = 1;
                output_file = optarg;
                break;
            case 'f':
                if(argIndex != 2){
                    printf("Third argument must be filter type. Exiting\n");
                    exit(1);
                }
                if((optarg[0] != 'b' && optarg[0] != 'c') || strlen(optarg) != 1){
                    printf("Invalid filter type argument. Exiting\n\n");
                    exit(1);
                }
                if(fFlag == 1){
                    printf("Extra argument for filter type: %s.  Additional argument discarded.\n", optarg);
                    argIndex++;
                    break;
                }
                argIndex++;
                fFlag = 1;
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
    if(input_file != NULL){
        length = strlen(input_file);
        //read in bmp input file
        if(length >= 5
           && (strcmp(&input_file[length - 4], ".bmp") == 0)
           && (access(input_file, F_OK) != -1)){
            printf("Input: %s\n", input_file);
            FILE* iFile = fopen(input_file, "rb");
            //read a bmp header from file
            input_bmp_header = (BMP_Header*)malloc(sizeof(BMP_Header));
            readBMPHeader(iFile, input_bmp_header);
            //read a dib header from file
            input_dib_header = (DIB_Header*)malloc(sizeof(DIB_Header));
            readDIBHeader(iFile, input_dib_header);
            //read a bmp pixel array from file
            fseek(iFile, input_bmp_header->offset_pixel_array, SEEK_SET);
            inputArr = (Pixel**)malloc(input_dib_header->width * sizeof(Pixel*));
            for(i = 0; i < input_dib_header->width; i++){
                inputArr[i] = (Pixel*) malloc(input_dib_header->height * sizeof(Pixel));
            }
            readPixelsBMP(iFile, inputArr, input_dib_header->width, input_dib_header->height);
            fclose(iFile);
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
    int height = input_dib_header->height;
    int width = input_dib_header->width;

    //apply a blur box filter
    if(filter_type == 'b'){
        //calculate size of chunks
        //int chunk = width / THREAD_COUNT;
        //create a stencil
        Stencil* stencil = (Stencil*)malloc(sizeof(Stencil));
        Pixel* pixel = (Pixel*)malloc(sizeof(Pixel));
        for(i = 0; i < height; i++){
            for(j = 0; j < width; j++){
                //if pixel is upper left corner
                if(i == 0 && j == 0){
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
                    stencil->pixel[1][1].red = inputArr[i][j].red;
                    stencil->pixel[1][1].green = inputArr[i][j].green;
                    stencil->pixel[1][1].blue = inputArr[i][j].blue;
                    stencil->pixel[1][2].red = inputArr[i][j + 1].red;
                    stencil->pixel[1][2].green = inputArr[i][j + 1].green;
                    stencil->pixel[1][2].blue = inputArr[i][j + 1].blue;
                    stencil->pixel[2][1].red = inputArr[i + 1][j].red;
                    stencil->pixel[2][1].green = inputArr[i + 1][j].green;
                    stencil->pixel[2][1].blue = inputArr[i + 1][j].blue;
                    stencil->pixel[2][2].red = inputArr[i + 1][j + 1].red;
                    stencil->pixel[2][2].green = inputArr[i + 1][j + 1].green;
                    stencil->pixel[2][2].blue = inputArr[i + 1][j + 1].blue;
                    inputArr[0][0] = *make_blurry_pixel(stencil, pixel, 4);
                }
                //if pixel is upper right corner
                if(i == 0 && j == width - 1){
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
                    stencil->pixel[1][0].red = inputArr[i][j - 1].red;
                    stencil->pixel[1][0].green = inputArr[i][j - 1].green;
                    stencil->pixel[1][0].blue = inputArr[i][j - 1].blue;
                    stencil->pixel[1][1].red = inputArr[i][j].red;
                    stencil->pixel[1][1].green = inputArr[i][j].green;
                    stencil->pixel[1][1].blue = inputArr[i][j].blue;
                    stencil->pixel[2][0].red = inputArr[i + 1][j - 1].red;
                    stencil->pixel[2][0].green = inputArr[i + 1][j - 1].green;
                    stencil->pixel[2][0].blue = inputArr[i + 1][j - 1].blue;
                    stencil->pixel[2][1].red = inputArr[i + 1][j].red;
                    stencil->pixel[2][1].green = inputArr[i + 1][j].green;
                    stencil->pixel[2][1].blue = inputArr[i + 1][j].blue;
                    inputArr[0][width - 1] = *make_blurry_pixel(stencil, pixel, 4);
                }
                //if pixel is bottom left corner
                if(i == height - 1 && j == 0){
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
                    stencil->pixel[0][1].red = inputArr[i - 1][j].red;
                    stencil->pixel[0][1].green = inputArr[i - 1][j].green;
                    stencil->pixel[0][1].blue = inputArr[i - 1][j].blue;
                    stencil->pixel[0][2].red = inputArr[i - 1][j + 1].red;
                    stencil->pixel[0][2].green = inputArr[i - 1][j + 1].green;
                    stencil->pixel[0][2].blue = inputArr[i - 1][j + 1].blue;
                    stencil->pixel[1][1].red = inputArr[i][j].red;
                    stencil->pixel[1][1].green = inputArr[i][j].green;
                    stencil->pixel[1][1].blue = inputArr[i][j].blue;
                    stencil->pixel[1][2].red = inputArr[i][j + 1].red;
                    stencil->pixel[1][2].green = inputArr[i][j + 1].green;
                    stencil->pixel[1][2].blue = inputArr[i][j + 1].blue;
                    inputArr[height - 1][0] = *make_blurry_pixel(stencil, pixel, 4);
                }
                //if pixel is bottom right corner
                if(i == height - 1 && j == width - 1){
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
                    stencil->pixel[0][0].red = inputArr[i - 1][j - 1].red;
                    stencil->pixel[0][0].green = inputArr[i - 1][j - 1].green;
                    stencil->pixel[0][0].blue = inputArr[i - 1][j - 1].blue;
                    stencil->pixel[0][1].red = inputArr[i - 1][j].red;
                    stencil->pixel[0][1].green = inputArr[i - 1][j].green;
                    stencil->pixel[0][1].blue = inputArr[i - 1][j].blue;
                    stencil->pixel[1][0].red = inputArr[i][j - 1].red;
                    stencil->pixel[1][0].green = inputArr[i][j - 1].green;
                    stencil->pixel[1][0].blue = inputArr[i][j - 1].blue;
                    stencil->pixel[1][1].red = inputArr[i][j].red;
                    stencil->pixel[1][1].green = inputArr[i][j].green;
                    stencil->pixel[1][1].blue = inputArr[i][j].blue;
                    inputArr[height - 1][width - 1] = *make_blurry_pixel(stencil, pixel, 4);
                }
                //if pixel is on the left edge
                if(i != 0 && i != height - 1 && j == 0) {
                    for (k = 0; k < 3; k++) {
                        stencil->pixel[k][0].red = 0;
                        stencil->pixel[k][0].green = 0;
                        stencil->pixel[k][0].blue = 0;
                    }
                    stencil->pixel[0][1].red = inputArr[i - 1][j].red;
                    stencil->pixel[0][1].green = inputArr[i - 1][j].green;
                    stencil->pixel[0][1].blue = inputArr[i - 1][j].blue;
                    stencil->pixel[0][2].red = inputArr[i - 1][j + 1].red;
                    stencil->pixel[0][2].green = inputArr[i - 1][j + 1].green;
                    stencil->pixel[0][2].blue = inputArr[i - 1][j + 1].blue;
                    stencil->pixel[1][1].red = inputArr[i][j].red;
                    stencil->pixel[1][1].green = inputArr[i][j].green;
                    stencil->pixel[1][1].blue = inputArr[i][j].blue;
                    stencil->pixel[1][2].red = inputArr[i][j + 1].red;
                    stencil->pixel[1][2].green = inputArr[i][j + 1].green;
                    stencil->pixel[1][2].blue = inputArr[i][j + 1].blue;
                    stencil->pixel[2][1].red = inputArr[i + 1][j].red;
                    stencil->pixel[2][1].green = inputArr[i + 1][j].green;
                    stencil->pixel[2][1].blue = inputArr[i + 1][j].blue;
                    stencil->pixel[2][2].red = inputArr[i + 1][j + 1].red;
                    stencil->pixel[2][2].green = inputArr[i + 1][j + 1].green;
                    stencil->pixel[2][2].blue = inputArr[i + 1][j + 1].blue;
                    inputArr[i][0] = *make_blurry_pixel(stencil, pixel, 6);
                }
                //if pixel is on the right edge
                if(i != 0 && i != height - 1 && j == width - 1) {
                    for (k = 0; k < 3; k++) {
                        stencil->pixel[k][2].red = 0;
                        stencil->pixel[k][2].green = 0;
                        stencil->pixel[k][2].blue = 0;
                    }
                    stencil->pixel[0][0].red = inputArr[i - 1][j - 1].red;
                    stencil->pixel[0][0].green = inputArr[i - 1][j - 1].green;
                    stencil->pixel[0][0].blue = inputArr[i - 1][j - 1].blue;
                    stencil->pixel[0][1].red = inputArr[i - 1][j].red;
                    stencil->pixel[0][1].green = inputArr[i - 1][j].green;
                    stencil->pixel[0][1].blue = inputArr[i - 1][j].blue;
                    stencil->pixel[1][0].red = inputArr[i][j - 1].red;
                    stencil->pixel[1][0].green = inputArr[i][j - 1].green;
                    stencil->pixel[1][0].blue = inputArr[i][j - 1].blue;
                    stencil->pixel[1][1].red = inputArr[i][j].red;
                    stencil->pixel[1][1].green = inputArr[i][j].green;
                    stencil->pixel[1][1].blue = inputArr[i][j].blue;
                    stencil->pixel[2][0].red = inputArr[i + 1][j - 1].red;
                    stencil->pixel[2][0].green = inputArr[i + 1][j - 1].green;
                    stencil->pixel[2][0].blue = inputArr[i + 1][j - 1].blue;
                    stencil->pixel[2][1].red = inputArr[i + 1][j].red;
                    stencil->pixel[2][1].green = inputArr[i + 1][j].green;
                    stencil->pixel[2][1].blue = inputArr[i + 1][j].blue;
                    inputArr[i][width - 1] = *make_blurry_pixel(stencil, pixel, 6);
                }
                //if pixel is on upper edge
                if(i == 0 && j != 0 && j != width - 1) {
                    for (k = 0; k < 3; k++) {
                        stencil->pixel[0][k].red = 0;
                        stencil->pixel[0][k].green = 0;
                        stencil->pixel[0][k].blue = 0;
                    }
                    stencil->pixel[1][0].red = inputArr[i][j - 1].red;
                    stencil->pixel[1][0].green = inputArr[i][j - 1].green;
                    stencil->pixel[1][0].blue = inputArr[i][j - 1].blue;
                    stencil->pixel[1][1].red = inputArr[i][j].red;
                    stencil->pixel[1][1].green = inputArr[i][j].green;
                    stencil->pixel[1][1].blue = inputArr[i][j].blue;
                    stencil->pixel[1][2].red = inputArr[i][j + 1].red;
                    stencil->pixel[1][2].green = inputArr[i][j + 1].green;
                    stencil->pixel[1][2].blue = inputArr[i][j + 1].blue;
                    stencil->pixel[2][0].red = inputArr[i + 1][j - 1].red;
                    stencil->pixel[2][0].green = inputArr[i + 1][j - 1].green;
                    stencil->pixel[2][0].blue = inputArr[i + 1][j - 1].blue;
                    stencil->pixel[2][1].red = inputArr[i + 1][j].red;
                    stencil->pixel[2][1].green = inputArr[i + 1][j].green;
                    stencil->pixel[2][1].blue = inputArr[i + 1][j].blue;
                    stencil->pixel[2][2].red = inputArr[i + 1][j + 1].red;
                    stencil->pixel[2][2].green = inputArr[i + 1][j + 1].green;
                    stencil->pixel[2][2].blue = inputArr[i + 1][j + 1].blue;
                    inputArr[0][j] = *make_blurry_pixel(stencil, pixel, 6);
                }
                //if pixel is on bottom edge
                if(i == height - 1 && j != 0 && j != width - 1) {
                    for (k = 0; k < 3; k++) {
                        stencil->pixel[2][k].red = 0;
                        stencil->pixel[2][k].green = 0;
                        stencil->pixel[2][k].blue = 0;
                    }
                    stencil->pixel[0][0].red = inputArr[i - 1][j - 1].red;
                    stencil->pixel[0][0].green = inputArr[i - 1][j - 1].green;
                    stencil->pixel[0][0].blue = inputArr[i - 1][j - 1].blue;
                    stencil->pixel[0][1].red = inputArr[i - 1][j].red;
                    stencil->pixel[0][1].green = inputArr[i - 1][j].green;
                    stencil->pixel[0][1].blue = inputArr[i - 1][j].blue;
                    stencil->pixel[0][2].red = inputArr[i - 1][j + 1].red;
                    stencil->pixel[0][2].green = inputArr[i - 1][j + 1].green;
                    stencil->pixel[0][2].blue = inputArr[i - 1][j + 1].blue;
                    stencil->pixel[1][0].red = inputArr[i][j - 1].red;
                    stencil->pixel[1][0].green = inputArr[i][j - 1].green;
                    stencil->pixel[1][0].blue = inputArr[i][j - 1].blue;
                    stencil->pixel[1][1].red = inputArr[i][j].red;
                    stencil->pixel[1][1].green = inputArr[i][j].green;
                    stencil->pixel[1][1].blue = inputArr[i][j].blue;
                    stencil->pixel[1][2].red = inputArr[i][j + 1].red;
                    stencil->pixel[1][2].green = inputArr[i][j + 1].green;
                    stencil->pixel[1][2].blue = inputArr[i][j + 1].blue;
                    inputArr[height - 1][j] = *make_blurry_pixel(stencil, pixel, 6);
                }
                //if pixel is not on border
                if(i != 0 && i != height - 1 && j != 0 && j != width - 1) {
                    stencil->pixel[0][0].red = inputArr[i - 1][j - 1].red;
                    stencil->pixel[0][0].green = inputArr[i - 1][j - 1].green;
                    stencil->pixel[0][0].blue = inputArr[i - 1][j - 1].blue;
                    stencil->pixel[0][1].red = inputArr[i - 1][j].red;
                    stencil->pixel[0][1].green = inputArr[i - 1][j].green;
                    stencil->pixel[0][1].blue = inputArr[i - 1][j].blue;
                    stencil->pixel[0][2].red = inputArr[i - 1][j + 1].red;
                    stencil->pixel[0][2].green = inputArr[i - 1][j + 1].green;
                    stencil->pixel[0][2].blue = inputArr[i - 1][j + 1].blue;
                    stencil->pixel[1][0].red = inputArr[i][j - 1].red;
                    stencil->pixel[1][0].green = inputArr[i][j - 1].green;
                    stencil->pixel[1][0].blue = inputArr[i][j - 1].blue;
                    stencil->pixel[1][1].red = inputArr[i][j].red;
                    stencil->pixel[1][1].green = inputArr[i][j].green;
                    stencil->pixel[1][1].blue = inputArr[i][j].blue;
                    stencil->pixel[1][2].red = inputArr[i][j + 1].red;
                    stencil->pixel[1][2].green = inputArr[i][j + 1].green;
                    stencil->pixel[1][2].blue = inputArr[i][j + 1].blue;
                    stencil->pixel[2][0].red = inputArr[i + 1][j - 1].red;
                    stencil->pixel[2][0].green = inputArr[i + 1][j - 1].green;
                    stencil->pixel[2][0].blue = inputArr[i + 1][j - 1].blue;
                    stencil->pixel[2][1].red = inputArr[i + 1][j].red;
                    stencil->pixel[2][1].green = inputArr[i + 1][j].green;
                    stencil->pixel[2][1].blue = inputArr[i + 1][j].blue;
                    stencil->pixel[2][2].red = inputArr[i + 1][j + 1].red;
                    stencil->pixel[2][2].green = inputArr[i + 1][j + 1].green;
                    stencil->pixel[2][2].blue = inputArr[i + 1][j + 1].blue;
                    inputArr[i][j] = *make_blurry_pixel(stencil, pixel, 9);
                }
            }
        }
    }
    //apply a Swiss cheese filter
    else{
        //determine smallest dimension of input
        int smallest = 0;
        if(width < height)
            smallest = width;
        else smallest = height;
    }
    //produce an output file
    if(oFlag == 1){
        FILE* oFile = fopen(output_file, "wb");
        writeBMPHeader(oFile, input_bmp_header);
        writeDIBHeader(oFile, input_dib_header);
        writePixelsBMP(oFile, inputArr, input_dib_header->width, input_dib_header->height);
        fclose(oFile);
        for(i = 0; i < input_dib_header->width; i++)
            free(inputArr[i]);
        free(inputArr);
        free(input_bmp_header);
        free(input_dib_header);
        printf("Output: %s\n", output_file);
    }
    return 0;
}

Pixel* make_blurry_pixel(Stencil* stencil, Pixel* pixel, int numPixels) {
    printf("num pixels: %d\n", numPixels);
    printf("red: %d\n", pixel->red);
    printf("green: %d\n", pixel->green);
    printf("blue: %d\n", pixel->blue);
    int i, j, rSum = 0, gSum = 0, bSum = 0;
    for(i = 0; i < 2; i++)
        for(j = 0; j < 2; j++){
            rSum += stencil->pixel[i][j].red;
            gSum += stencil->pixel[i][j].green;
            bSum += stencil->pixel[i][j].blue;
        }

    pixel->red = rSum / numPixels;
    pixel->green = gSum / numPixels;
    pixel->blue = bSum / numPixels;
    printf("after\n");
    printf("red: %d\n", pixel->red);
    printf("green: %d\n", pixel->green);
    printf("blue: %d\n", pixel->blue);
    return pixel;
}
