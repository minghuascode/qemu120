/*
 ** selectserver.c -- a cheezy multiperson chat server
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

#define NUM_PORTS (4)
//#define NUM_CONNS (8)
static struct soc_listeners_s {
    int listener;
    int connector;
    int used;
} soc_listeners[NUM_PORTS];

static struct soc_fds_s {
    fd_set master;
    fd_set listeners;
    int    fdmax;
} soc_fds = {0,0,0};

static int poll_init();
static int poll_select();

int main(void)
{
    int rc = poll_init();
    if ( rc ) {
        return 1;
    }
    
    for (;;) {
        rc = poll_select();
        if ( rc ) {
            return 2;
        }
    }
    return 0;
}

static int poll_init()
{
    int listnr;       // listening socket descriptor
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, rv;

    struct addrinfo hints, *ai, *p;
    
    memset(&soc_listeners, 0, sizeof(soc_listeners));

    FD_ZERO(&soc_fds.master);       // clear the master and temp sets
    FD_ZERO(&soc_fds.listeners);    // clear the master and temp sets
    soc_fds.fdmax = 0;
    char portstr[30];
    memset(portstr, 0, 30);

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
        printf("Opened socket %d: port %s\n", i, portstr);
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
    for(i = 0; i <= soc_fds.fdmax; i++) {
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
            FD_SET(newfd, &soc_fds.master); // add to master set
            if (newfd > soc_fds.fdmax) {    // keep track of the max
                soc_fds.fdmax = newfd;
            }
            printf("selectserver: new connection from %s on socket %d\n",
                    inet_ntop(remoteaddr.ss_family,
                    get_in_addr((struct sockaddr*)&remoteaddr),
                    remoteIP, INET6_ADDRSTRLEN), newfd);
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
            continue;
        }
        // we got some data from a client
        for(j = 0; j <= soc_fds.fdmax; j++) {
            // send to everyone!
            if ( FD_ISSET(j, &soc_fds.master) && 
                // except the listener and ourselves
                    !FD_ISSET(j, &soc_fds.listeners) && j != i ) {
                if (send(j, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    } // END looping through file descriptors
    return 0;
}


