#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>


int main(int argc, char **argv)
{
#if 0
    int *vs = NULL;
    int sizes[] = { 128, 64, 32, 256, 1024, 1025, 1026, 2048, 8 };
    int N = sizeof(sizes) / sizeof(sizes[0]);

    for (int i = 0; i < N; ++i) {
        printf("calloc: %d\n", sizes[i]);
        // vs = realloc(vs, sizeof(*vs) * sizes[i]);
        int *ts = malloc(sizeof(*ts) * sizes[i]);
        free(vs);
        vs = ts;
        printf("size  : %zu\n", malloc_usable_size(vs));
    }

    free(vs);
#endif

#if 1
    int N = 1000;
    int M = 1 << 20;
    int i, sum, *ns, *is, *vs = NULL;
    srand(42);
    if (argc == 2) {
        N = atoi(argv[1]);
    }
    ns = calloc(sizeof(int), N);
    is = calloc(sizeof(int), N);
    for (i = 0; i < N; ++i) {
        ns[i] = (rand() % M) + 1;
        is[i] = rand() % ns[i];
    }

    for (i = 0; i < N; ++i) {
        vs = realloc(vs, sizeof(int) * ns[i]);
        /* touch the memory to force page fault */
        memset(vs, 0xaa, sizeof(int) * ns[i]);
        vs[is[i]] = i;
    }

    sum = 0;
    for (i = 0; i < ns[N-1]; ++i) {
        sum += vs[i];
    }

    free(vs);
    free(is);
    free(ns);
    printf("sum = %d\n", sum);
    return sum != 0 ? 0 : 1;
#endif
}
