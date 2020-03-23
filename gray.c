#include <stdio.h>
#include <stdint.h>
#include <immintrin.h>

int next_increment(uint64_t *c, int *radices, int n)
{
    int k = 0;
    int mod = 1;

    (*c)++;

    for (k = 0; k < n; k++) {
        mod *= radices[k];
        if (*c % mod)
            break;
    }

    if (k == n)
        return -1;

    return k;

}

int next_increment_bin(uint64_t *c, int n)
{
    (*c)++;
    *c = *c % (1 << n);
    int res = _tzcnt_u64(*c);

    if (*c == 0) {
        return n - 1;
    } else if (res >= n) {
        return -1;
    }

    return res;
}


//int main()
//{
//    uint64_t c = 0;
//    int i;
//    int radices[3] = {2, 4, 3};
//
//    i = next_increment(c, radices, 3);
//    //i = next_increment_bin(c, 30);
//    while (i != -1) {
//        printf("%d\n", i);
//        i = next_increment(c, radices, 3);
//    //    i = next_increment_bin(c, 30);
//    }
//
//}
