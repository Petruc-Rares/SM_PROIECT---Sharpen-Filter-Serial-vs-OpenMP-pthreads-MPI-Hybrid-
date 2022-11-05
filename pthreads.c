#include "include/bmp_header.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include <string.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/time.h>

#define MASTER 0
#define BUFSIZE 100

bmp *photoSharpened;
bmp *photo;

int no_threads;

void readFileHeader(bmp_fileheader * header, FILE * in) {
  fread(&(header -> fileMarker1), 1, 1, in);
  fread(&(header -> fileMarker2), 1, 1, in);
  fread(&(header -> bfSize), 1, 4, in);
  fread(&(header -> unused1), 1, 2, in);
  fread(&(header -> unused2), 1, 2, in);
  fread(&(header -> imageDataOffset), 1, 4, in);
  return;
}

// function used for reading infoHeaders fields
void readInfoHeader(bmp_infoheader * infoHeader, FILE * in) {
  fread(&(infoHeader -> biSize), 1, 4, in);
  fread(&(infoHeader -> width), 1, 4, in);
  fread(&(infoHeader -> height), 1, 4, in);
  fread(&(infoHeader -> planes), 1, 2, in);
  fread(&(infoHeader -> bitPix), 1, 2, in);
  fread(&(infoHeader -> biCompression), 1, 4, in);
  fread(&(infoHeader -> biSizeImage), 1, 4, in);
  fread(&(infoHeader -> biXPelsPerMeter), 1, 4, in);
  fread(&(infoHeader -> biYPelsPerMeter), 1, 4, in);
  fread(&(infoHeader -> biClrUsed), 1, 4, in);
  fread(&(infoHeader -> biClrImportant), 1, 4, in);
  return;
}

// function used for allocation of a pointer of bmp_fileheader type
bmp_fileheader * alloc_fileheader() {
  bmp_fileheader * header = (bmp_fileheader *)calloc(1,
                                                       sizeof(bmp_fileheader));
    if (!header) {
        printf("Alocarea fileheader a failuit\n");
        exit(1);
    }
            
  return header;
}

// function used for allocation of a pointer of bmp_infoheader type
bmp_infoheader * alloc_infoheader() {
  bmp_infoheader * header = (bmp_infoheader *)calloc(1,
                                                       sizeof(bmp_infoheader));
    
    if (!header) {
        printf("Alocarea infoheader a failuit\n");
        exit(1);
    }

  return header;
}

// function that checks how many bytes are required
// for each row of the photo in order
// to respect the BMP format
int getPaddingBytes(bmp_infoheader * infoHeader) {
    signed int aux = infoHeader -> width * 3;
    while (aux % 4 != 0) aux++;
    int paddingBytes = aux - infoHeader -> width * 3;
    return paddingBytes;
}

// functio used for copy of header
void copyHeader(bmp_fileheader * header, FILE * in , FILE * out) {
  signed char c;
  unsigned int k = 0;
  while (k < (header -> imageDataOffset)) {
    c = fgetc(in);
    fputc(c, out);
    k++;
  }
  return;
}

bmp *read_bmp_file(char *input_filename) {
    photo = (bmp *) calloc(1, sizeof(bmp));
    if (!photo) {
        printf("Allocation of bmp structure failed\n");
        exit(1);
    }

    FILE *in = fopen(input_filename, "rb");
    if (in == NULL) {
        printf("file opening failed\n");
        exit(1);
    }

    photo->fileHeader = alloc_fileheader(); // this is the file header
    readFileHeader(photo->fileHeader, in);

    photo->infoHeader = alloc_infoheader(); // this is the info header
    readInfoHeader(photo->infoHeader, in);

    photo->paddingBytes = getPaddingBytes(photo->infoHeader);

    signed int heightPhoto = photo->infoHeader->height;
    signed int widthPhoto = photo->infoHeader->width;

    photo->photoData = (Pixel **) calloc(heightPhoto, sizeof(Pixel *));
    if (!photo->photoData) {
        printf("Allocation failed for rows of photoData\n");
        exit(1);
    }

    for (int i = 0; i < heightPhoto; i++) {
        photo->photoData[i] = (Pixel *) calloc(widthPhoto, sizeof(Pixel));
        if (!photo->photoData[i]) {
            printf("Allocation failed for columns of photoData");
            exit(1);
        }
    }

    unsigned char buffer;

    for (int i = 0; i < heightPhoto; i++) {
        for (int j = 0; j < widthPhoto; j++) {
            fread(&(photo->photoData[i][j].blue), sizeof(unsigned char), 1, in);
            fread(&(photo->photoData[i][j].green), sizeof(unsigned char), 1, in);
            fread(&(photo->photoData[i][j].red), sizeof(unsigned char), 1, in);
        }

        if (!(photo->paddingBytes)) continue;

        for (int j = widthPhoto; j < widthPhoto + photo->paddingBytes; j++) {
            fread(&buffer, sizeof(unsigned char), 1, in);
        }
    }

    return photo;
}

void write_bmp_file(bmp* photo, char *output_filename) {
    FILE *out = fopen(output_filename, "wb");

    unsigned int i = photo->fileHeader->imageDataOffset;

    fwrite(photo->fileHeader, sizeof(*(photo->fileHeader)), 1, out);
    fwrite(photo->infoHeader, sizeof(*(photo->infoHeader)), 1, out);

    signed int heightPhoto = photo->infoHeader->height;
    signed int widthPhoto = photo->infoHeader->width;

    unsigned char padding_byte = 0;

    for (int i = 0; i < heightPhoto; i++) {
        for (int j = 0; j < widthPhoto; j++) {
            fwrite(&(photo->photoData[i][j].blue), sizeof(unsigned char), 1, out);
            fwrite(&(photo->photoData[i][j].green), sizeof(unsigned char), 1, out);
            fwrite(&(photo->photoData[i][j].red), sizeof(unsigned char), 1, out);
        }

        if (!(photo->paddingBytes)) continue;

        for (int j = 0; j < photo->paddingBytes; j++) {
            fwrite(&padding_byte, sizeof(unsigned char), 1, out);
        }

    }
    
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

void *func_thread(void *arg) {
    int filter[3][3] = {
                        -1, -1, -1,
                        -1, 9, -1,
                        -1, -1, -1
                        };    

    int filterWidth = 3;
    int filterHeight = 3;

    Pixel **image = photo->photoData;

    signed int heightPhoto = photoSharpened->infoHeader->height;
    signed int widthPhoto = photoSharpened->infoHeader->width;

    int thread_id = *(int *) arg;

    int start = thread_id * (double) heightPhoto / no_threads;
    int end = min((thread_id + 1) * (double) heightPhoto / no_threads, heightPhoto);

    for (int i = start; i < end; i++) {
        for (int j = 0; j < widthPhoto; j++) {
            int blue = 0, green = 0, red = 0;

            for (int k = 0; k < filterHeight; k++) {
                for (int q = 0; q < filterWidth; q++) {
                    int imageX = (j - filterWidth / 2 + q + widthPhoto) % widthPhoto;
                    int imageY = (i - filterHeight / 2 + k + heightPhoto) % heightPhoto;

                    red += image[imageY][imageX].red * filter[k][q];
                    green += image[imageY][imageX].green * filter[k][q];
                    blue += image[imageY][imageX].blue * filter[k][q];
                }
            }

            photoSharpened->photoData[i][j].red = min(max(red, 0), 255) ;
            photoSharpened->photoData[i][j].green = min(max(green, 0), 255);
            photoSharpened->photoData[i][j].blue = min(max(blue, 0), 255);   
        }
    }
}


bmp *apply_sharpen(bmp *photo, int argc, char *argv[]) {
    photoSharpened = (bmp *) calloc(1, sizeof(bmp));
    if (!photoSharpened) {
        printf("Allocation of bmp structure failed\n");
        exit(1);
    }    

    photoSharpened->fileHeader = photo->fileHeader;
    photoSharpened->infoHeader = photo->infoHeader;
    photoSharpened->paddingBytes = photo->paddingBytes;

    signed int heightPhoto = photoSharpened->infoHeader->height;
    signed int widthPhoto = photoSharpened->infoHeader->width;

    photoSharpened->photoData = (Pixel **) calloc(heightPhoto, sizeof(Pixel *));
    if (!photoSharpened->photoData) {
        printf("Allocation failed for rows of photoData\n");
        exit(1);
    }

    for (int i = 0; i < heightPhoto; i++) {
        photoSharpened->photoData[i] = (Pixel *) calloc(widthPhoto, sizeof(Pixel));
        if (!photoSharpened->photoData[i]) {
            printf("Allocation failed for columns of photoData");
            exit(1);
        }
    }


    /* THREAD CREATION AND JOIN BELOW WAS DONE
    USING https://ocw.cs.pub.ro/courses/apd/laboratoare/01 */
    pthread_t threads[no_threads];
    int r;
    long id;
    void *status;
    long arguments[no_threads];
 
    struct timeval current_time_create, current_time_join;
    gettimeofday(&current_time_create, NULL);
    
    for (id = 0; id < no_threads; id++) {
        arguments[id] = id;
        r = pthread_create(&threads[id], NULL, func_thread, (void *) &arguments[id]);
 
        if (r) {
            printf("Eroare la crearea thread-ului %ld\n", id);
            exit(-1);
        }
    }
 
    for (id = 0; id < no_threads; id++) {
        r = pthread_join(threads[id], &status);
 
        if (r) {
            printf("Eroare la asteptarea thread-ului %ld\n", id);
            exit(-1);
        }
    }

    // getTimeOfDay

    gettimeofday(&current_time_join, NULL);

    double time_taken = current_time_join.tv_sec - current_time_create.tv_sec + (current_time_join.tv_usec - current_time_create.tv_usec) * 1e-6;

    printf("pthreads function took %lf seconds\n", time_taken);

    
    return photoSharpened;
}

char *source_folder = "photos/";
char *destination_folder = "sharpened_photos/";

char source_path[BUFSIZE];
char destination_path[BUFSIZE];

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Invalid usage. Correct is: ./serial <image_name> <no_threads>\n");
        return 1;
    }

    no_threads = atoi(argv[2]);

    memset(source_path, 0, BUFSIZE);
    memset(destination_path, 0, BUFSIZE);

    strcat(source_path, source_folder);
    strcat(source_path, argv[1]);

    bmp *photo = read_bmp_file(source_path);

    bmp *new_photo = apply_sharpen(photo, argc, argv);
  
    strcat(destination_path, destination_folder);
    // without ".bmp"
    strncat(destination_path, argv[1], strlen(argv[1]) - 4);
    strcat(destination_path, "_pthreads_sharpen");
    strcat(destination_path, ".bmp");

    write_bmp_file(new_photo, destination_path);
}