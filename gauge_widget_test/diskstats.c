#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define SYS_BLOCK_PATH "/sys/block/"
#define ULL unsigned long long

/* 
   This program performs the following steps:

   1. Reads data from the file "/sys/block/%s/stat", which contains the 
      number of completed read and write operations
   2. Waits for 10 seconds, then reads the same file again and calculate the 
      read and write operations per second 
   3. Determines a suitable unit divider to scale the numbers into the range 
      0 to 1023:
        - Dividers are powers of 2: k (2^10), m (2^20), g (2^30), etc.
        - If no divider is used, the unit is represented by an asterisk (*).
   4. Prints the results in the following format:
        device[0] = nvme0n1   // Name of the block device
        read[0] = 0           // Number of read operations
        write[0] = 1          // Number of write operations
        unit[0] = *           // Unit for scaling the numbers (e.g., k, m, g, or *)
 */

// Function to check if the device is a disk (starts with 'sd', 'nvme', etc.)
int is_disk(const char *device) {
    return (strncmp(device, "sd", 2) == 0 || strncmp(device, "nvme", 4) == 0 || strncmp(device, "mmcblk", 6) == 0);
}


int drive_count;
char *names[100];
unsigned long long reads[100], writes[100];





// Function to retrieve and print the statistics for a block device
void read_sys_block_stats(int offs) {
    char stat_path[256];
    FILE *f;
    unsigned long long read_ios, write_ios, read_sectors, write_sectors;


    for( int i = 0 ; i < drive_count; i++ ) {
	    char *device = names[i]; 
	        
	    // Construct the path to the device's stat file
	    snprintf(stat_path, sizeof(stat_path), "/sys/block/%s/stat", device);

	    // Open the stat file
	    f = fopen(stat_path, "r");
	    if (!f) {
		    perror("fopen");
		    continue;
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

	    // printf("  Read Sectors:   %llu\n", read_sectors);
	    // printf("  Write Sectors:  %llu\n\n", write_sectors);
	    // Close the file
	    fclose(f);
	    reads[offs+i] = read_ios;
	    writes[offs+i] = write_ios;
    }
}

int main()
{
    DIR *dir;
    struct dirent *ent;
    drive_count=0;
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
		    names[ drive_count++ ] = strdup(ent->d_name);
            }
        }
        closedir(dir);
    } else {
	    perror("opendir");
	    return -1;
    }

    
    read_sys_block_stats(0);
    sleep(10);
    read_sys_block_stats( drive_count );

    const char *units[] = { "*", "K", "M", "G", "T", "P", "E", "Z", "Y" };
    int unit = 0;

    for(int i=0;i<drive_count;i++) {	
	    printf("device[%d]=%s\n", i, names[i] );
	    

	    ULL mx;
	    ULL rd = (reads[i + drive_count ] - reads[i]) / 10;
	    ULL wr =  (writes[i + drive_count ] - writes[i]) / 10;
	    if( rd < wr ) mx = wr; else mx=rd;

	    for(unit=0;unit < 8; unit ++) {		    
		    if( mx < 1024 ) break;
		    rd /= 1024;
		    wr /= 1024;
		    mx /= 1024;
	    }
	    	    
	    printf("read[%d]=%llu\n", i,  rd  );
	    printf("write[%d]=%llu\n",i,  wr  );
	    printf("unit[%d]=%s\n", i, units[unit] );
    }
    
    

    
    return 0;
}
