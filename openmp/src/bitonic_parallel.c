/*********************************************************************
 *
 * https://www.cs.duke.edu/courses/fall08/cps196.1/Pthreads/bitonic.c
 *
 *********************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define LENGTH 8

FILE *fin, *fout;

char *strings;
long int N;

unsigned long int powersOfTwo[] = {1,        2,        4,        8,         16,        32,        64,        128,
                                   256,      512,      1024,     2048,      4096,      8192,      16384,     32768,
                                   65536,    131072,   262144,   524288,    1048576,   2097152,   4194304,   8388608,
                                   16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824};

#define ASCENDING 1
#define DESCENDING 0

void openfiles(char *filePath)
{
    fin = fopen(filePath, "r");
    // fin = fopen("data/in_1024.in", "r");
    if (fin == NULL)
    {
        perror("fopen fin");
        exit(EXIT_FAILURE);
    }

    fout = fopen("sort.out", "w");
    if (fout == NULL)
    {
        perror("fopen fout");
        exit(EXIT_FAILURE);
    }
}

void closefiles(void)
{
    fclose(fin);
    fclose(fout);
}

void compare(int i, int j, int dir)
{
    if (dir == (strcmp(strings + i * LENGTH, strings + j * LENGTH) > 0))
    {
        char t[LENGTH];
        strcpy(t, strings + i * LENGTH);
        strcpy(strings + i * LENGTH, strings + j * LENGTH);
        strcpy(strings + j * LENGTH, t);
    }
}

void bitonicMerge(int lo, int cnt, int dir)
{
    if (cnt > 1)
    {
        int k = cnt / 2;

        // Paralelizando a comparação entre os pares
        #pragma omp parallel for schedule(dynamic)
        for (int i = lo; i < lo + k; i++)
            compare(i, i + k, dir);

        bitonicMerge(lo, k, dir);
        bitonicMerge(lo + k, k, dir);
    }
}

void recBitonicSort(int lo, int cnt, int dir)
{
    if (cnt > 1)
    {
        int k = cnt / 2;

        // Executando as duas metades em paralelo
        #pragma omp parallel sections
        {
            #pragma omp section
            recBitonicSort(lo, k, ASCENDING);

            #pragma omp section
            recBitonicSort(lo + k, k, DESCENDING);
        }

        bitonicMerge(lo, cnt, dir);
    }
}

void BitonicSort()
{
    recBitonicSort(0, N, ASCENDING);
}

int main(int argc, char **argv)
{
    long int i;
    if (argc < 2)
    {
        printf("Usage: %s <input_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    printf("Input file: %s\n", argv[1]);
    openfiles(argv[1]);

    fscanf(fin, "%ld", &N);

    if (N > 1073741824 || powersOfTwo[(int)log2(N)] != N)
    {
        printf("%ld is not a valid number: power of 2 or less than 1073741824!\n", N);
        exit(EXIT_FAILURE);
    }

    strings = (char *)calloc(N, LENGTH);
    if (strings == NULL)
    {
        perror("malloc strings");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++)
        fscanf(fin, "%s", strings + (i * LENGTH));

    double initTime = omp_get_wtime();
    BitonicSort();
    printf("Total time = %.5lf\n", omp_get_wtime() - initTime);

    for (i = 0; i < N; i++)
        fprintf(fout, "%s\n", strings + (i * LENGTH));

    free(strings);
    closefiles();

    return EXIT_SUCCESS;
}
