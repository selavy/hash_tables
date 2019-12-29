#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

#define GIVEBACK

#define min(a, b) ((a) < (b) ? (a) : (b))

int main(int argc, char **argv)
{
    int N = 50;
    int M = 1 << 20;
    int i, sz, sum, *ns, *is, *ts, *vs = NULL;
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

    sz = 0;
    for (i = 0; i < N; ++i) {
#ifndef GIVEBACK
        if (ns[i] > sz) {
#endif
            ts = malloc(sizeof(int) * ns[i]);
            /* assert(ts != NULL); */
            /* assert(vs != NULL || sz == 0); */
            /* TODO: elideded null checks */
            /* assert(i == 0 || sz == ns[i-1]); */
            memcpy(ts, vs, sizeof(int) * min(sz, ns[i]));
            free(vs);
            vs = ts;
#ifndef GIVEBACK
        }
#endif
        sz = ns[i];
        memset(vs, 0xaa, sizeof(int) * sz);
        /* touch the memory to force page fault */
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
}
