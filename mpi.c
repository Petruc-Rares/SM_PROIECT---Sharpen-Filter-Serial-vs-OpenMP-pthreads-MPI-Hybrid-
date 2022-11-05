#include "include/bmp_header.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include "mpi.h"
#include <string.h>
#include <stddef.h>

#define MASTER 0
#define BUFSIZE 100

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
    bmp *photo = (bmp *) calloc(1, sizeof(bmp));
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


bmp *apply_sharpen(bmp *photo, int argc, char *argv[]) {
    bmp *photoSharpened = (bmp *) calloc(1, sizeof(bmp));
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


    int filter[3][3] = {
                        -1, -1, -1,
                        -1, 9, -1,
                        -1, -1, -1
                        };    

    int filterWidth = 3;
    int filterHeight = 3;

    Pixel **image = photo->photoData;


    int numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

     /* create a type for struct Pixel */
    const int nitems= NO_CHANNELS;
    int blocklengths[NO_CHANNELS] = {1,1, 1};

    MPI_Datatype types[NO_CHANNELS] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};
    MPI_Datatype mpi_pixel_type;
    MPI_Aint     offsets[NO_CHANNELS];

    offsets[0] = offsetof(Pixel, blue);
    offsets[1] = offsetof(Pixel, green);
    offsets[2] = offsetof(Pixel, red);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_pixel_type);
    MPI_Type_commit(&mpi_pixel_type);

    int start = rank * (double) heightPhoto / numtasks;
    int end = min((rank + 1) * (double) heightPhoto / numtasks, heightPhoto);

    //printf("rank: %d deals with start: %d, end: %d\n", rank, start, end);

    
    double t1, t2; 
    t1 = MPI_Wtime(); 

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
    

    if (rank == MASTER) {
        // vector cu cele no_tasks - 1 alte calculatoare, fiecare avand pozitia corespunzatoare de start
        int buffer[widthPhoto];
        int *crt_index_by_rank = (int *) malloc((numtasks - 1) * sizeof(int));

        for (int i = 0; i < numtasks - 1; i++) {
            crt_index_by_rank[i] = (i + 1) * (double) heightPhoto / numtasks;
        }

        MPI_Status status;

        for (int i = end; i < heightPhoto; i++) {
            MPI_Recv(buffer, widthPhoto, mpi_pixel_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int receivedFrom = status.MPI_SOURCE;
            int index = crt_index_by_rank[receivedFrom - 1]++;
            memcpy(photoSharpened->photoData[index], buffer, widthPhoto*sizeof(Pixel));
        }
    }
    else {
        for (int i = start; i < end; i++) {
            MPI_Send(photoSharpened->photoData[i], widthPhoto, mpi_pixel_type, MASTER, 0, MPI_COMM_WORLD);
        }
    }

    t2 = MPI_Wtime(); 

    if (rank == MASTER) {
        printf("mpi function took %lf seconds\n", (t2 - t1));
    }

    MPI_Finalize();
    
    if (rank == 0) {
        return photoSharpened;
    } else {
        //printf("bebe\n");
    }

    return NULL;
}

char *source_folder = "photos/";
char *destination_folder = "sharpened_photos/";

char source_path[BUFSIZE];
char destination_path[BUFSIZE];

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Invalid usage. Correct is: ./serial <image_name>\n");
        return 1;
    }

    memset(source_path, 0, BUFSIZE);
    memset(destination_path, 0, BUFSIZE);

    strcat(source_path, source_folder);
    strcat(source_path, argv[1]);

    bmp *photo = read_bmp_file(source_path);

    bmp *new_photo = apply_sharpen(photo, argc, argv);

    strcat(destination_path, destination_folder);
    // without ".bmp"
    strncat(destination_path, argv[1], strlen(argv[1]) - 4);
    strcat(destination_path, "_mpi_sharpen");
    strcat(destination_path, ".bmp");

    // write only the photo that was built in master
    if (new_photo) {
        write_bmp_file(new_photo, destination_path);
    }
}