#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/wireless.h>
#include <unistd.h>
#include <errno.h>
#include "sensorreg.h"
#include "mls.h"
#include "var5.h"

// Function to print the wireless statistics for a given interface
void print_wireless_stats(int sockfd, const char* ifname) {
    int sockfd;
    struct iw_statistics stats;
    struct iwreq req;

    memset(&req, 0, sizeof(struct iwreq));
    strncpy(req.ifr_name, ifname, IFNAMSIZ);
    req.u.data.pointer = &stats;
    req.u.data.length = sizeof(struct iw_statistics);
    req.u.data.flags = 1;  // Required flag for wireless statistics

    // Create a socket for communication with the network interface
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Could not create socket");
        return;
    }

    // Perform the ioctl call to get the wireless statistics
    if (ioctl(sockfd, SIOCGIWSTATS, &req) == -1) {
        if (errno == EOPNOTSUPP) {
            // Not a wireless interface
            close(sockfd);
            return;
        } else {
            perror("Error performing SIOCGIWSTATS");
            close(sockfd);
            return;
        }
    }

    // Print the statistics
    printf("Interface: %s\n", ifname);
    printf("Link Quality: %d/%d\n", stats.qual.qual, 100);  // Max quality is usually 100
    printf("Signal Level: %d dBm\n", (signed char)stats.qual.level);
    printf("Noise Level: %d dBm\n", (signed char)stats.qual.noise);
    printf("Updated: %d\n\n", stats.qual.updated);

    
    
    // Close the socket
    close(sockfd);
}

// Function to get all interfaces and print wireless statistics
void get_all_interfaces() {
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
    for (i = 0; i < n_interfaces; i++) {
        strncpy(ifr.ifr_name, it[i].ifr_name, IFNAMSIZ);

        // Check if this is a wireless interface using SIOCGIWNAME
        if (ioctl(sockfd, SIOCGIWNAME, &ifr) != -1) {
            // If it's a wireless interface, print the statistics
            print_wireless_stats(it[i].ifr_name);
        } 
    }

    // Close the socket
    close(sockfd);
}

int main() {
	m_init();
	mvar_init();
	get_all_interfaces();

	mvar_destruct();
	m_destruct();
	return 0;
}
