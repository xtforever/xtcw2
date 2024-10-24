#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/wireless.h>
#include <unistd.h>
#include <errno.h>


/* exported variables:
  sensor.wlanstat.interface   string[]
  sensor.wlanstat.quality     integer[]  0-100
*/

// Function to print the wireless statistics for a given interface
static void print_wireless_stats(int sockfd, const char* ifname, int num ) {

    struct iw_statistics stats;
    struct iwreq req;

    memset(&req, 0, sizeof(struct iwreq));
    strncpy(req.ifr_name, ifname, IFNAMSIZ);
    req.u.data.pointer = &stats;
    req.u.data.length = sizeof(struct iw_statistics);
    req.u.data.flags = 1;  // Required flag for wireless statistics


    // Perform the ioctl call to get the wireless statistics
    if (ioctl(sockfd, SIOCGIWSTATS, &req) == -1) {
            perror("Error performing SIOCGIWSTATS");
	    return;	
    }

    // Print the statistics
    printf("interface[%d]=%s\n", num, ifname);
    printf("linkquality[%d]=%d\n", num, stats.qual.qual );  // Max quality is usually 100

    int n;
    n = 100 + (signed char)stats.qual.level;
    if( n > 100 ) n=100; else if(n < 0 ) n=0;    
    printf("signallevel[%d]=%d\n", num, n);

    n = 100 + (signed char)stats.qual.noise;
    if( n > 100 ) n=100; else if(n < 0 ) n=0;    
    printf("noiselevel[%d]=[%d]\n", num, n );
}

// Function to get all interfaces and print wireless statistics
static void get_all_interfaces() {
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int sockfd, i;

    // Create a socket to list all interfaces
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket error");
        exit(EXIT_FAILURE);
    }

    // Populate the ifconf structure to get all available interfaces
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) == -1) {
        perror("ioctl SIOCGIFCONF");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Loop through all interfaces
    struct ifreq* it = ifc.ifc_req;
    int n_interfaces = ifc.ifc_len / sizeof(struct ifreq);
    int num=0;
    for (i = 0; i < n_interfaces; i++) {
        strncpy(ifr.ifr_name, it[i].ifr_name, IFNAMSIZ);

        // Check if this is a wireless interface using SIOCGIWNAME
        if (ioctl(sockfd, SIOCGIWNAME, &ifr) != -1) {
            // If it's a wireless interface, print the statistics
		print_wireless_stats(sockfd, it[i].ifr_name, num++ );
        } 
    }

    // Close the socket
    close(sockfd);
}


void wlanstat_run( void )
{
	get_all_interfaces();	
}


#ifdef WITH_WLANSTAT_MAIN

int main()
{
	get_all_interfaces();
	return 0;
}

#endif
