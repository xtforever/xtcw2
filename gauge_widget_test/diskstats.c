#include <stdio.h>
#include <dirent.h>
#include <string.h>

#define SYS_BLOCK_PATH "/sys/block/"

// Function to check if the device is a disk (starts with 'sd', 'nvme', etc.)
int is_disk(const char *device) {
    return (strncmp(device, "sd", 2) == 0 || strncmp(device, "nvme", 4) == 0 || strncmp(device, "mmcblk", 6) == 0);
}

// Function to retrieve and print the statistics for a block device
void print_sys_block_stats(const char *device, int num) {
    char stat_path[256];
    FILE *f;
    unsigned long long read_ios, write_ios, read_sectors, write_sectors;

    // Construct the path to the device's stat file
    snprintf(stat_path, sizeof(stat_path), "/sys/block/%s/stat", device);

    // Open the stat file
    f = fopen(stat_path, "r");
    if (!f) {
        perror("fopen");
        return;
    }

    // Read the statistics from the file
    // The format of /sys/block/{device}/stat file is:
    // 1. Reads completed successfully
    // 2. Reads merged
    // 3. Sectors read
    // 4. Time spent reading (ms)
    // 5. Writes completed
    // 6. Writes merged
    // 7. Sectors written
    // 8. Time spent writing (ms)
    // Only reading the relevant fields for I/O counts and sectors
    fscanf(f, "%llu %*u %llu %*u %llu %*u %llu",
           &read_ios, &read_sectors, &write_ios, &write_sectors);

    // Print the statistics
    printf("device[%d]=%s\n", num, device);
    printf("read[%d]=%llu\n", num, read_ios);
    printf("write[%d]=%llu\n",num, write_ios);
    // printf("  Read Sectors:   %llu\n", read_sectors);
    // printf("  Write Sectors:  %llu\n\n", write_sectors);
    // Close the file
    fclose(f);
}

int main() {


	
    DIR *dir;
    struct dirent *ent;
    int num=0;
    
    // Open the /sys/block directory to list block devices
    if ((dir = opendir(SYS_BLOCK_PATH)) != NULL) {
        // Iterate over each entry in /sys/block
        while ((ent = readdir(dir)) != NULL) {
            // Skip the '.' and '..' entries
            if (ent->d_name[0] == '.') {
                continue;
            }

            // Check if the device is a disk (starts with 'sd', 'nvme', 'mmcblk', etc.)
            if (is_disk(ent->d_name)) {
                // Print stats for the disk
		    print_sys_block_stats(ent->d_name, num++);
            }
        }
        closedir(dir);
    } else {
        perror("opendir");
        return -1;
    }

    return 0;
}
