/*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/
/* how to set up a new macvlan interface on a mac addr: 
 *  ip link add link eth0 type macvlan name newname 
 *  ip link set dev newname addr 12:22:33:44:44:44
 */

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#if 0
#define MY_DEST_MAC0 0x12
#define MY_DEST_MAC1 0x22
#define MY_DEST_MAC2 0x33
#define MY_DEST_MAC3 0x44
#define MY_DEST_MAC4 0x55
#define MY_DEST_MAC5 0x03
#else /* multicast */
#define MY_DEST_MAC0 0x01
#define MY_DEST_MAC1 0x02
#define MY_DEST_MAC2 0x03
#define MY_DEST_MAC3 0x04
#define MY_DEST_MAC4 0x05
#define MY_DEST_MAC5 0x06
#endif

#define DEFAULT_IF "eth0"
#define BUF_SIZ ETH_FRAME_LEN

int main(int argc, char *argv[])
{
    int sockfd;
    struct ifreq if_idx;
    struct ifreq if_mac;
    int tx_len = 0;
    char sendbuf[BUF_SIZ];
    struct ether_header *eh = (struct ether_header *) sendbuf;
    //struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
    struct sockaddr_ll socket_address;
    char ifName[IFNAMSIZ] = {0};

    /* Get interface name */
    int dorx = 0;
    if (argc > 2)
        dorx = atol(argv[2]);
    if (argc > 1)
        strncpy(ifName, argv[1], IFNAMSIZ-1);
    else
        strncpy(ifName, DEFAULT_IF, IFNAMSIZ-1);

    /* Open RAW socket to send on */
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)/*IPPROTO_RAW*/)) == -1) {
        perror("socket");
    }

    /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");
    /* Get the MAC address of the interface to send on */
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        perror("SIOCGIFHWADDR");

    if (dorx) {
        struct ifreq if_multi;
        memset(&if_multi, 0, sizeof(if_multi));
        strncpy(if_multi.ifr_name, ifName, IFNAMSIZ-1);
        if_multi.ifr_ifru.ifru_hwaddr.sa_data[0] = 0x01;
        if_multi.ifr_ifru.ifru_hwaddr.sa_data[1] = 0x02;
        if_multi.ifr_ifru.ifru_hwaddr.sa_data[2] = 0x03;
        if_multi.ifr_ifru.ifru_hwaddr.sa_data[3] = 0x04;
        if_multi.ifr_ifru.ifru_hwaddr.sa_data[4] = 0x05;
        if_multi.ifr_ifru.ifru_hwaddr.sa_data[5] = 0x06;
        if (ioctl(sockfd, SIOCADDMULTI, &if_multi) < 0)
            perror("SIOCADDMULTI");
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, 
                            strnlen(ifName,IFNAMSIZ-1)) < 0)
            perror("SO_BINDTODEVICE");
        int length = 0; /*length of the received frame*/ 
        length = recvfrom(sockfd, sendbuf, BUF_SIZ, 0, NULL, NULL);
        printf("Received %d\n", length);
        if (length == -1) {  
            
        }
        return 0;
    }
    
    /* Construct the Ethernet header */
    memset(sendbuf, 0, BUF_SIZ);
    /* Ethernet header */
    eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
    eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
    eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
    eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
    eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
    eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
    
    eh->ether_dhost[0] = MY_DEST_MAC0;
    eh->ether_dhost[1] = MY_DEST_MAC1;
    eh->ether_dhost[2] = MY_DEST_MAC2;
    eh->ether_dhost[3] = MY_DEST_MAC3;
    eh->ether_dhost[4] = MY_DEST_MAC4;
    eh->ether_dhost[5] = MY_DEST_MAC5;
    
    /* Ethertype field */
    eh->ether_type = htons(ETH_P_IP);
    tx_len += sizeof(struct ether_header);

    /* Packet data */
    sendbuf[tx_len++] = 0xde;
    sendbuf[tx_len++] = 0xad;
    sendbuf[tx_len++] = 0xbe;
    sendbuf[tx_len++] = 0xef;

    /* Index of the network device */
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* Address length*/
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    socket_address.sll_addr[0] = MY_DEST_MAC0;
    socket_address.sll_addr[1] = MY_DEST_MAC1;
    socket_address.sll_addr[2] = MY_DEST_MAC2;
    socket_address.sll_addr[3] = MY_DEST_MAC3;
    socket_address.sll_addr[4] = MY_DEST_MAC4;
    socket_address.sll_addr[5] = MY_DEST_MAC5;

    /* Send packet */
    if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, 
            sizeof(struct sockaddr_ll)) < 0)
        printf("Send failed\n");

    return 0;
}

