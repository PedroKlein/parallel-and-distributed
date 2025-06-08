#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH 8

int task_threshold = 2048;
#define DEFAULT_CSV_MODE 0

#define OUTPUT_DIR "output/"
#define INPUT_DIR "data/"
#define DEFAULT_INPUT_FILE "in_16777216.in"

FILE *fin, *fout;
char *strings;
long int N;

unsigned long int powersOfTwo[] = {1,        2,        4,        8,         16,        32,        64,        128,
                                   256,      512,      1024,     2048,      4096,      8192,      16384,     32768,
                                   65536,    131072,   262144,   524288,    1048576,   2097152,   4194304,   8388608,
                                   16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824};

#define ASCENDING 1
#define DESCENDING 0

int csv_mode = DEFAULT_CSV_MODE;

void parseCommandLineArguments(int argc, char **argv, char *input_file, char *sort_method)
{
    for (int arg = 1; arg < argc; arg++)
    {
        if (strcmp(argv[arg], "-i") == 0 && arg + 1 < argc)
        {
            snprintf(input_file, 256, INPUT_DIR "%s", argv[arg + 1]);
            arg++;
        }
        else if (strcmp(argv[arg], "-t") == 0 && arg + 1 < argc)
        {
            task_threshold = atoi(argv[arg + 1]);
            arg++;
        }
        else if (strcmp(argv[arg], "-csv") == 0)
        {
            csv_mode = 1;
        }
        else if (strcmp(argv[arg], "-sort") == 0 && arg + 1 < argc)
        {
            strncpy(sort_method, argv[arg + 1], 32);
            sort_method[31] = '\0'; // Ensure null-termination
            arg++;
        }
        else
        {
            fprintf(stderr, "Usage: %s [-i input_file] [-t task_threshold] [-csv] [-sort method]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

void openfiles(char *input_file)
{
    fin = fopen(input_file, "r");
    if (fin == NULL)
    {
        perror("fopen fin");
        fprintf(stderr, "Error opening input file: %s\n", input_file);
        exit(EXIT_FAILURE);
    }

    char output_file[256];
    snprintf(output_file, sizeof(output_file), OUTPUT_DIR "%s.out",
             strrchr(input_file, '/') ? strrchr(input_file, '/') + 1 : input_file);
    fout = fopen(output_file, "w");
    if (fout == NULL)
    {
        perror("fopen fout");
        fprintf(stderr, "Error opening output file: %s\n", output_file);
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
        int i;
        for (i = lo; i < lo + k; i++)
            compare(i, i + k, dir);
        bitonicMerge(lo, k, dir);
        bitonicMerge(lo + k, k, dir);
    }
}

void bitonicMergeParallel(int lo, int cnt, int dir)
{
    if (cnt > 1)
    {
        int k = cnt / 2;
        // #pragma omp parallel for // this makes it slower
        for (int i = lo; i < lo + k; i++)
        {
            compare(i, i + k, dir);
        }

#pragma omp task firstprivate(lo, k) if (cnt > task_threshold)
        {
            bitonicMergeParallel(lo, k, dir);
        }
#pragma omp task firstprivate(lo, k) if (cnt > task_threshold)
        {
            bitonicMergeParallel(lo + k, k, dir);
        }
#pragma omp taskwait
    }
}

void recBitonicSort(int lo, int cnt, int dir)
{
    if (cnt > 1)
    {
        int k = cnt / 2;
        recBitonicSort(lo, k, ASCENDING);
        recBitonicSort(lo + k, k, DESCENDING);
        bitonicMerge(lo, cnt, dir);
    }
}

void recBitonicSortParallel(int lo, int cnt, int dir)
{
    if (cnt > 1)
    {
        int k = cnt / 2;
#pragma omp task firstprivate(lo, k) if (cnt > task_threshold)
        {
            recBitonicSortParallel(lo, k, ASCENDING);
        }
#pragma omp task firstprivate(lo, k) if (cnt > task_threshold)
        {
            recBitonicSortParallel(lo + k, k, DESCENDING);
        }
#pragma omp taskwait
        bitonicMergeParallel(lo, cnt, dir);
    }
}

void bitonicSort()
{
    recBitonicSort(0, N, ASCENDING);
}

void bitonicSortParallel()
{
#pragma omp parallel
    {
#pragma omp single
        {
            recBitonicSortParallel(0, N, ASCENDING);
        }
    }
}

void merge(int low, int mid, int high)
{
    int n1 = mid - low;
    int n2 = high - mid;
    char *temp = (char *)malloc((n1 + n2) * LENGTH);
    if (temp == NULL)
    {
        perror("malloc temp");
        exit(EXIT_FAILURE);
    }
    int i = 0, j = 0;
    while (i < n1 && j < n2)
    {
        if (strcmp(strings + (low + i) * LENGTH, strings + (mid + j) * LENGTH) <= 0)
        {
            memcpy(temp + (i + j) * LENGTH, strings + (low + i) * LENGTH, LENGTH);
            i++;
        }
        else
        {
            memcpy(temp + (i + j) * LENGTH, strings + (mid + j) * LENGTH, LENGTH);
            j++;
        }
    }
    while (i < n1)
    {
        memcpy(temp + (i + j) * LENGTH, strings + (low + i) * LENGTH, LENGTH);
        i++;
    }
    while (j < n2)
    {
        memcpy(temp + (i + j) * LENGTH, strings + (mid + j) * LENGTH, LENGTH);
        j++;
    }
    memcpy(strings + low * LENGTH, temp, (n1 + n2) * LENGTH);
    free(temp);
}

void recMergeSort(int low, int high)
{
    if (high - low < 2)
        return;
    int mid = (low + high) / 2;
    recMergeSort(low, mid);
    recMergeSort(mid, high);
    merge(low, mid, high);
}

void recMergeSortParallel(int low, int high)
{
    if (high - low < 2)
        return;
    int mid = (low + high) / 2;
#pragma omp task firstprivate(low, mid) if ((high - low) > task_threshold)
    {
        recMergeSortParallel(low, mid);
    }
#pragma omp task firstprivate(mid, high) if ((high - low) > task_threshold)
    {
        recMergeSortParallel(mid, high);
    }
#pragma omp taskwait
    merge(low, mid, high);
}

void mergeSort()
{
    recMergeSort(0, N);
}

void mergeSortParallel()
{
#pragma omp parallel
    {
#pragma omp single
        {
            recMergeSortParallel(0, N);
        }
    }
}

void sort(const char *method)
{
    if (strcmp(method, "bitonic") == 0)
    {
        bitonicSort();
    }
    else if (strcmp(method, "bitonic_parallel") == 0)
    {
        bitonicSortParallel();
    }
    else if (strcmp(method, "mergesort") == 0)
    {
        mergeSort();
    }
    else if (strcmp(method, "mergesort_parallel") == 0)
    {
        mergeSortParallel();
    }
    else
    {
        fprintf(stderr, "Unknown sorting method: %s\n", method);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    long int i;
    char input_file[256] = INPUT_DIR DEFAULT_INPUT_FILE;
    char sort_method[32] = "bitonic_parallel"; // Default sorting method

    parseCommandLineArguments(argc, argv, input_file, sort_method);

    openfiles(input_file);

    fscanf(fin, "%ld", &N);
    if (N > 1073741824 || powersOfTwo[(int)log2(N)] != N)
    {
        printf("%ld is not a valid number: must be a power of 2 and less than 1073741824!\n", N);
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

    double startTime = omp_get_wtime();
    sort(sort_method);
    double total_time = omp_get_wtime() - startTime;

    if (csv_mode)
    {
        int omp_threads = omp_get_max_threads();
        printf("input_file,N,task_threshold,omp_num_threads,total_time\n");
        printf("%s,%ld,%d,%d,%.6lf\n", input_file, N, task_threshold, omp_threads, total_time);
    }
    else
    {
        printf("Total time = %.6lf seconds\n", total_time);
    }

    for (i = 0; i < N; i++)
        fprintf(fout, "%s\n", strings + (i * LENGTH));

    free(strings);
    closefiles();
    return EXIT_SUCCESS;
}
