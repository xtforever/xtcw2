#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#define BUFFER_SIZE 1024
#define CPU_FIELDS 10
#define RESOLUTION 10000
enum stats {  USER, NICE, SYSTEM, IDLE, IOWAIT, IRQ, SOFTIRQ, STEAL, GUEST, GUEST_NICE };

	
// Structure to store CPU time data
struct cpu {
	unsigned long long val[CPU_FIELDS];
};

/* scan table */
struct cpu *scan_table(int num)
{	
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    struct cpu *cpu_usage_table = calloc( num, sizeof( struct cpu )); 	
    for(int i=0;i<num;i++) {
	    struct cpu *cpu_usage = & cpu_usage_table[i];
	    int err = fscanf(fp, "%*[^ ] %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu%*[\n]",
			     cpu_usage->val, cpu_usage->val+1,cpu_usage->val+2,cpu_usage->val+3,
			     cpu_usage->val+4,cpu_usage->val+5,cpu_usage->val+6,cpu_usage->val+7,
			     cpu_usage->val+8,cpu_usage->val+9 );
    

	    if( err != 10 ) { perror("parse error"); exit(1); }
    }
    fclose(fp);
    return cpu_usage_table; 		
}
// Function to calculate CPU usage percentage
void calc_cpu_usage(int line, struct cpu *prev, struct cpu *curr ) {

	unsigned long long total_diff=0, diff[CPU_FIELDS] = { 0 };
	for(int i=0;i<CPU_FIELDS;i++) {
		diff[i] = curr->val[i] - prev->val[i];
		total_diff += diff[i];
	}
	
	long iowait = RESOLUTION  * ( diff[IOWAIT] * 1.0 / total_diff );
	printf("iowait[%d]=%ld\n", line, iowait );
	
	long idle_diff = diff[IDLE] + diff[STEAL] + diff[IOWAIT];
	long usage = (RESOLUTION  * 1.0 * (total_diff - idle_diff)  )  / total_diff;
	printf("usage[%d]=%ld\n", line, usage );

	long sys = RESOLUTION * diff[SYSTEM] * 1.0 / total_diff;
	printf("system[%d]=%ld\n", line, sys );
}




// Structure to store CPU time data
typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
} CPUUsage;

// Function to read CPU stats from /proc/stat
int read_cpu_usage(CPUUsage *cpu_usage, int core) {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int line = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (core == -1 && line == 0) {
            // Read total CPU usage
            sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &cpu_usage->user, &cpu_usage->nice, &cpu_usage->system,
                   &cpu_usage->idle, &cpu_usage->iowait, &cpu_usage->irq,
                   &cpu_usage->softirq, &cpu_usage->steal, &cpu_usage->guest,
                   &cpu_usage->guest_nice);
            break;
        } else if (core != -1 && line == core + 1) {
            // Read specific core usage
            sscanf(buffer, "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &core, &cpu_usage->user, &cpu_usage->nice, &cpu_usage->system,
                   &cpu_usage->idle, &cpu_usage->iowait, &cpu_usage->irq,
                   &cpu_usage->softirq, &cpu_usage->steal, &cpu_usage->guest,
                   &cpu_usage->guest_nice);
            break;
        }
        line++;
    }

    fclose(fp);
    return 0;
}

int main() {

    int num_cores = sysconf(_SC_NPROCESSORS_ONLN); 

    struct cpu *a1 = scan_table( num_cores + 1 );
    sleep(1); // Sleep for a second to calculate the usage difference
    struct cpu *a2 = scan_table( num_cores + 1);

    printf("ncores=%d\n", num_cores );
    printf("max=%d\n", RESOLUTION );
    for (int i = 0; i < num_cores + 1; i++) {
	    calc_cpu_usage(i, &a1[i], &a2[i]);
    }

    free(a1); free(a2);
    return 0;
}
