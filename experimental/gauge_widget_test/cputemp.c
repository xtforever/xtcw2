#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#define TEMP_PATH "/sys/class/thermal/"

// Function to read the temperature from the thermal zone file
int read_temp(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Failed to open temperature file");
        return -1;
    }

    int temp_millidegrees;
    if (fscanf(fp, "%d", &temp_millidegrees) != 1) {
        perror("Failed to read temperature value");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    // Return temperature in Celsius (converting from millidegrees)
    return temp_millidegrees / 1000;
}

int main() {
    struct dirent *entry;
    DIR *dir = opendir(TEMP_PATH);

    if (dir == NULL) {
        perror("Failed to open /sys/class/thermal");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Look for directories like thermal_zoneX
        if (strncmp(entry->d_name, "thermal_zone", 12) == 0) {
            char temp_file_path[256];
            snprintf(temp_file_path, sizeof(temp_file_path), "%s%s/temp", TEMP_PATH, entry->d_name);

            int temp = read_temp(temp_file_path);
            if (temp >= 0) {
                printf("%s: %d°C\n", entry->d_name, temp);
            }
        }
    }

    closedir(dir);
    return 0;
}
