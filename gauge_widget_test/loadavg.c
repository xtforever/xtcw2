#include <stdio.h>
#include <stdlib.h>

int main() {
    double loadavg[3];

    // Get load averages for 1, 5, and 15 minutes
    if (getloadavg(loadavg, 3) != -1) {
        printf("System Load Averages:\n");
        printf("1 minute:  %.2f\n", loadavg[0]);
        printf("5 minutes: %.2f\n", loadavg[1]);
        printf("15 minutes: %.2f\n", loadavg[2]);
    } else {
        perror("getloadavg failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
