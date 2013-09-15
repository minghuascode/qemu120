/*
 ** selectserver.c -- a cheezy multiperson chat server
 * original file from beej.us
 * modified to open and maintain multiple listening ports, 
 * and to serve stats on the last port.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define SOC_PORT (9030)   // port we're listening on

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void *get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_port);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_port);
}

#define NUM_PORTS (5)
#define STATS_PORT_INDEX (NUM_PORTS-1)
#define CATCH_PORT_INDEX (NUM_PORTS)
//#define NUM_CONNS (8)
static struct soc_listeners_s {
  int listener;  /* fd */
  int connector; /* fd of a connection */
  int used;      /* connected */
  uint32_t raddr; uint16_t rport;
  uint32_t bytes_rx_total, bytes_tx_total, bytes_rx_conn, bytes_tx_conn, 
           opencount, closecount;
} soc_listeners[NUM_PORTS+1];

static struct soc_fds_s {
  fd_set master;
  fd_set listeners;
  int    fdmax;
} soc_fds = {0,0,0};

static int listnr_idx_by_datafd(int datafd) {
    int i;
    for (i=0; i<NUM_PORTS; i++) {
        if (soc_listeners[i].used &&soc_listeners[i].connector == datafd ) {
            return i;
        }
    }
    return i; /* catch-all port */
}
static int listnr_idx_by_listnrfd(int listnrfd) {
    int i;
    for (i=0; i<NUM_PORTS; i++) {
        if ( soc_listeners[i].listener == listnrfd ) {
            return i;
        }
    }
    return i; /* catch-all port */
}
#define is_stats_idx(idx) ( idx==STATS_PORT_INDEX )
#define is_catch_idx(idx) ( idx==CATCH_PORT_INDEX )
static int listnr_opened_by_listnrfd(int listnrfd) {
    int idx = listnr_idx_by_listnrfd(listnrfd);
    if ( is_catch_idx(idx) ) {
        return 0;
    } else {
        return soc_listeners[idx].used;
    }
}
static int listnr_opened_by_datafd(int datafd) {
    int idx = listnr_idx_by_datafd(datafd);
    if ( is_catch_idx(idx) ) {
        return 0;
    } else {
        return soc_listeners[idx].used;
    }
}
static int listnr_datafd_by_listnrfd(int listnrfd) {
    int idx = listnr_idx_by_listnrfd(listnrfd);
    if ( is_catch_idx(idx) ) {
        return 0;
    } else if ( soc_listeners[idx].used ) {
        return soc_listeners[idx].connector;
    } else {
        return 0;
    }
}
static int listnr_close_by_listnrfd(int listnrfd) {
    int idx = listnr_idx_by_listnrfd(listnrfd);
    if ( is_catch_idx(idx) || soc_listeners[idx].used ) {
        soc_listeners[idx].connector = 0;
        soc_listeners[idx].used = 0;
        soc_listeners[idx].closecount ++;
    }
    return 0;
}
static int listnr_close_by_datafd(int datafd) {
    int idx = listnr_idx_by_datafd(datafd);
    if ( is_catch_idx(idx) || soc_listeners[idx].used ) {
        soc_listeners[idx].connector = 0;
        soc_listeners[idx].used = 0;
        soc_listeners[idx].closecount ++;
    }
    return 0;
}
static int listnr_open(int listnrfd, int datafd, uint32_t ra, uint16_t rp) {
    int idx = listnr_idx_by_listnrfd(listnrfd);
    if ( soc_listeners[idx].used ) {
        idx = CATCH_PORT_INDEX;
    }
    soc_listeners[idx].connector = datafd;
    soc_listeners[idx].bytes_rx_conn = 0;
    soc_listeners[idx].bytes_tx_conn = 0;
    soc_listeners[idx].opencount ++;
    soc_listeners[idx].raddr = ra;
    soc_listeners[idx].rport = rp;
    if ( ! is_catch_idx(idx) ) {
        soc_listeners[idx].used = 1;
    }
    return 0;
}
static int listnr_incrx_by_datafd(int datafd, uint32_t cnt) {
    int idx = listnr_idx_by_datafd(datafd);
    if ( is_catch_idx(idx) || soc_listeners[idx].used ) {
        soc_listeners[idx].bytes_rx_conn += cnt;
        soc_listeners[idx].bytes_rx_total += cnt;
    }
    return 0;
}
static int listnr_inctx_by_datafd(int datafd, uint32_t cnt) {
    int idx = listnr_idx_by_datafd(datafd);
    if ( is_catch_idx(idx) || soc_listeners[idx].used ) {
        soc_listeners[idx].bytes_tx_conn += cnt;
        soc_listeners[idx].bytes_tx_total += cnt;
    }
    return 0;
}

static int poll_init();
static int poll_select();

int main(void)
{
    int rc = poll_init();
    if ( rc ) return 1;
    
    for (;;) {
      rc = poll_select();
      if ( rc ) break;
    }
    return 0;
}

static int poll_init()
{
    int listnr;     // listening socket descriptor
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, rv;

    struct addrinfo hints, *ai, *p;
    
    memset(&soc_listeners, 0, sizeof(soc_listeners));

    FD_ZERO(&soc_fds.master);    // clear the master and temp sets
    FD_ZERO(&soc_fds.listeners);    // clear the master and temp sets
    soc_fds.fdmax = 0;
    char portstr[30];
    memset(portstr, 0, 30);
    
    if ( NUM_PORTS < 2 ) {
        fprintf(stderr, "selectserver: must open minimum 2 ports\n");
        return 1;
    }

    for (i=0; i<NUM_PORTS; i++) {
        // get us a socket and bind it
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET; /*AF_UNSPEC;*/
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        snprintf(portstr, 29, "%d", SOC_PORT + i);
        if ((rv = getaddrinfo(NULL, portstr, &hints, &ai)) != 0) {
            fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
            return 1;
        }

        for(p = ai; p != NULL; p = p->ai_next) {
            listnr = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (listnr < 0) { 
                continue;
            }
            // lose the pesky "address already in use" error message
            setsockopt(listnr, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

            if (bind(listnr, p->ai_addr, p->ai_addrlen) < 0) {
                close(listnr);
                continue;
            }
            break;
        }

        // if we got here, it means we didn't get bound
        if (p == NULL) {
            fprintf(stderr, "selectserver: failed to bind\n");
            return 2;
        }

        freeaddrinfo(ai); // all done with this

       // listen
        if (listen(listnr, 10) == -1) {
            perror("listen");
            return 3;
        }

        // add the listener to the master set
        FD_SET(listnr, &soc_fds.master);
        FD_SET(listnr, &soc_fds.listeners);

        // keep track of the biggest file descriptor
        soc_fds.fdmax = (soc_fds.fdmax < listnr? listnr : soc_fds.fdmax); // so far, it's this one
        soc_listeners[i].listener = listnr;
        printf("Opened socket %d: port %s  %s\n", i, portstr, 
                is_stats_idx(i)?"for stats":"");
    }
    return 0;
}

static int poll_select()
{
    fd_set read_fds;  // temp file descriptor list for select()
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    char buf[256];    // buffer for client data
    int  nbytes;
    char remoteIP[INET6_ADDRSTRLEN];
    int i, j, rv;
    
    read_fds = soc_fds.master; // copy it
    struct timeval tv = { 0, 10*1000 };
    rv = select(soc_fds.fdmax+1, &read_fds, NULL, NULL, &tv);
    if ( rv == -EINTR || rv == 0 ) {
        return 0;
    } else if ( rv < 0 ) {
        perror("select");
        return 4;
    }

    // run through the existing connections looking for data to read
    for(i = 3; i <= soc_fds.fdmax; i++) {
        if ( ! FD_ISSET(i, &read_fds)) { 
            continue;
        }
        // we got one!!
        if ( FD_ISSET(i, &soc_fds.listeners) ) {
            // handle new connections
            addrlen = sizeof remoteaddr;
            newfd = accept(i, (struct sockaddr *)&remoteaddr, &addrlen);
            if (newfd == -1) {
                perror("accept");
                continue;
            }
            if ( listnr_opened_by_listnrfd(i) ) {
                int datafd = listnr_datafd_by_listnrfd(i);
                int idx    = listnr_idx_by_listnrfd(i);
                printf("selectserver: new connection on existing %d socket %d\n",
                       idx, datafd);
                close(datafd); // bye!
                FD_CLR(datafd, &soc_fds.master); // remove from master set
                listnr_close_by_listnrfd(i);
            }
            FD_SET(newfd, &soc_fds.master); // add to master set
            if (newfd > soc_fds.fdmax) {    // keep track of the max
                soc_fds.fdmax = newfd;
            }
            listnr_open(i, newfd, 
              ntohl(*((uint32_t*)get_in_addr((struct sockaddr*)&remoteaddr))),
              ntohs(*((uint16_t*)get_in_port((struct sockaddr*)&remoteaddr))));
            printf("selectserver: new connection from %s on socket %d port %u\n",
                   inet_ntop(remoteaddr.ss_family,
                              get_in_addr((struct sockaddr*)&remoteaddr),
                              remoteIP, INET6_ADDRSTRLEN), 
                   newfd, SOC_PORT + listnr_idx_by_datafd(newfd) );
            //printf("selectserver: addr %x port %u\n",
            // ntohl(*((uint32_t*)get_in_addr((struct sockaddr*)&remoteaddr))),
            // ntohs(*((uint16_t*)get_in_port((struct sockaddr*)&remoteaddr))));
                    /* note: getsockname() can get the same information */
            continue;
        }
        // handle data from a client
        if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
            // got error or connection closed by client
            if (nbytes == 0) {
                // connection closed
                printf("selectserver: socket %d hung up\n", i);
            } else {
                perror("recv");
            }
            close(i); // bye!
            FD_CLR(i, &soc_fds.master); // remove from master set
            listnr_close_by_datafd(i);
            continue;
        }
        // we got some data from a client
        listnr_incrx_by_datafd(i, nbytes);
        if ( is_stats_idx(listnr_idx_by_datafd(i)) ){
            char outbuf[256];
            int k, len;
            char hdrfmt[] = "\n idx port cnn lsnr sock "
                                "rxcount txcount rxtotal txtotal nopen nclose\n";
            char prtfmt[] =   " %3d %4d %2d  %3d  %3d  "
                                "%7u %7u %7u %7u  %3d   %3d - %u.%u.%u.%u %u\n";
            int anynonblank = 0;
            for (k=0; k<nbytes && k<4; k++) {
                if ( buf[k] != '\r' && buf[k] != '\n' ) {
                    anynonblank = 1;
                }
            }
            memset(outbuf, 0, 256);
            
            for(k = 3; k <= soc_fds.fdmax && anynonblank; k++) {
                len = snprintf(outbuf, 255, "   fd %2u  master %s  listener %s\n", k, 
                               FD_ISSET(k, &soc_fds.master)?"yes":" no", 
                               FD_ISSET(k, &soc_fds.listeners)?"yes":" no");
                send(i, outbuf, len, 0);
                listnr_inctx_by_datafd(i, len);
            }
            
            len = snprintf(outbuf, 255, "%s", hdrfmt);
            send(i, outbuf, len, 0);
            listnr_inctx_by_datafd(i, len);
            for (k=0; k<=NUM_PORTS; k++) {
                uint32_t ra = soc_listeners[k].raddr;
                len = snprintf(outbuf, 255, prtfmt, 
                               k, is_catch_idx(k)?0:SOC_PORT+k, 
                    soc_listeners[k].used, soc_listeners[k].listener, 
                    soc_listeners[k].connector, 
              soc_listeners[k].bytes_rx_conn, soc_listeners[k].bytes_tx_conn, 
              soc_listeners[k].bytes_rx_total, soc_listeners[k].bytes_tx_total, 
                    soc_listeners[k].opencount, soc_listeners[k].closecount, 
                    (uint8_t)((ra>>24)&0xff), (uint8_t)((ra>>16)&0xff), 
                    (uint8_t)((ra>>8)&0xff), (uint8_t)((ra>>0)&0xff), 
                    soc_listeners[k].rport);
                send(i, outbuf, len, 0);
                listnr_inctx_by_datafd(i, len);
            }
            continue;          
        }
        for(j = 3; j <= soc_fds.fdmax; j++) {
            // send to everyone!
            if ( FD_ISSET(j, &soc_fds.master) && 
                // except the listener and ourselves
                    !FD_ISSET(j, &soc_fds.listeners) && j != i ) {
                listnr_inctx_by_datafd(j, nbytes);
                if (send(j, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    } // END looping through file descriptors
    return 0;
}


