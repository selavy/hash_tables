#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(int argc, char** argv)
{
    int8_t x = -128; // 0xFFu;
    printf("x = %d\n", x);

    x = -x;
    printf("-x = %d\n", x);

    return 0;
}
