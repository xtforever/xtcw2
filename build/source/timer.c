
#include "timer.h"
#include <time.h>

/* get milliseconds */
int timer_get_ms(void)
{
    struct timespec t;

    clock_gettime( CLOCK_MONOTONIC_COARSE, &t );
    
    unsigned int ms = t.tv_nsec / 1000000U; /* 1E-9 to 1E-3 nano to milli */
    ms += t.tv_sec * 1000;
    return ms;
}

int timer_diff_ms(int timer_ms)
{
    unsigned int t1,t2;
    t2 = timer_ms;
    t1 = timer_get_ms();
    return t1-t2;
}
