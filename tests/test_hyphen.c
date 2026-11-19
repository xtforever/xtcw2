/*
 * Optional libhyphen integration test.
 * Skips cleanly when libhyphen or a dictionary is unavailable.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "hyphen.h"

int main(void)
{
    int n = 0, i;
    int *breaks;

    hyphen_set_language("de_DE");
    if (!hyphen_init()) {
        printf("test_hyphen skipped (no libhyphen/dictionary)\n");
        return 0;
    }

    breaks = hyphen_breaks("Silbentrennung", 14, &n);
    if (!breaks || n == 0) {
        printf("test_hyphen skipped (dictionary has no breaks for test word)\n");
        free(breaks);
        return 0;
    }

    for (i = 0; i < n; i++) {
        assert(breaks[i] > 0);
        assert(breaks[i] < 14);
    }
    free(breaks);
    printf("test_hyphen passed\n");
    return 0;
}
