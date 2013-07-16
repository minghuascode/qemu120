/*
 * Copyright (c) 2010 Putilov Andrey
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* change winsock.h to the following per beej.us */
#if (0) 
#include <winsock.h>
#else
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#endif

#include "md5.c"
#include "websocket.c"

#define BUF_LEN 0x1FF
#define PACKET_DUMP

int terminate = FALSE;

int client_worker(int cskt)
{
    static uint8_t buffer[BUF_LEN];
    size_t readed_length = 0;
    size_t out_len = BUF_LEN;
    int written = 0;
    enum ws_frame_type frame_type = WS_INCOMPLETE_FRAME;
    struct handshake hs;
    nullhandshake(&hs);

    // read openinig handshake
    while (frame_type == WS_INCOMPLETE_FRAME) {
        int readed = recv(cskt, buffer+readed_length, BUF_LEN-readed_length, 0);
        if ( readed == 0 ) {
            perror("Recv closed\n");
            return EXIT_FAILURE;
        }
        if (readed <0) {
            perror("Recv failed\n");
            return EXIT_FAILURE;
        }
        #ifdef PACKET_DUMP
                printf("In packet: readed len %d\n", readed);
                fwrite(buffer, 1, readed, stdout);
                printf("\n");
        #endif
        readed_length+= readed;
        assert(readed_length <= BUF_LEN);
        frame_type = ws_parse_handshake(buffer, readed_length, &hs);
        if (frame_type == WS_INCOMPLETE_FRAME && readed_length == BUF_LEN) {
            perror("Buffer too small\n");
            return EXIT_FAILURE;
        } else if (frame_type == WS_ERROR_FRAME) {
            perror("Error in incoming frame\n");
            return EXIT_FAILURE;
        }
    }
    assert(frame_type == WS_OPENING_FRAME);

    // if resource is right, generate answer handshake and send it
    if (strcmp(hs.resource, "/echo") != 0) {
        fprintf(stderr, "Resource is wrong:%s\n", hs.resource);
        perror("Resource is wrong\n");
        return EXIT_FAILURE;
    }
    out_len = BUF_LEN;
    ws_get_handshake_answer(&hs, buffer, &out_len);
    #ifdef PACKET_DUMP
            printf("Out packet:\n");
            fwrite(buffer, 1, out_len, stdout);
            printf("\n");
    #endif
    written = send(cskt, buffer, out_len, 0);
    if (written != out_len) {
        fprintf(stderr, "Written %d of %d\n", written, out_len);
        return EXIT_FAILURE;
    }
	
    // connect success!
    // read incoming packet and parse it;
    readed_length = 0;
    frame_type = WS_INCOMPLETE_FRAME;
    while (frame_type == WS_INCOMPLETE_FRAME) {
        int readed = recv(cskt, buffer+readed_length, BUF_LEN-readed_length, 0);
        if (readed == 0) {
            perror("Recv closed\n");
            return EXIT_FAILURE;
        }
        if (readed < 0) {
            fprintf(stderr, "Recv failed: %d\n", readed);
            perror("Recv failed\n");
            return EXIT_FAILURE;
        }
        #ifdef PACKET_DUMP
                printf("In packet:\n");
                fwrite(buffer, 1, readed, stdout);
                printf("\n");
        #endif
        readed_length+= readed;
        assert(readed_length <= BUF_LEN);
        size_t data_len;
        uint8_t *data;
        frame_type = ws_parse_input_frame(buffer, readed_length, &data, &data_len);
        if (frame_type == WS_INCOMPLETE_FRAME && readed_length == BUF_LEN) {
            perror("Buffer too small\n");
            return EXIT_FAILURE;
        } else if (frame_type == WS_CLOSING_FRAME) {
            send(cskt, "\xFF\x00", 2, 0); // send closing frame
            break;
        } else if (frame_type == WS_ERROR_FRAME) {
            perror("Error in incoming frame\n");
            return EXIT_FAILURE;
        } else if (frame_type == WS_TEXT_FRAME) {
            out_len = BUF_LEN;
            frame_type = ws_make_frame(data, data_len, buffer, &out_len, WS_TEXT_FRAME);
            if (frame_type != WS_TEXT_FRAME) {
                perror("Make frame failed\n");
                return EXIT_FAILURE;
            }
            #ifdef PACKET_DUMP
                    printf("Out packet:\n");
                    fwrite(buffer, 1, out_len, stdout);
                    printf("\n");
            #endif
            written = send(cskt, buffer, out_len, 0);
            if (written != out_len) {
                fprintf(stderr, "Written %d of %d\n", written, out_len);
                return EXIT_FAILURE;
            }
            readed_length = 0;
            frame_type = WS_INCOMPLETE_FRAME;
        }
    } // read/write cycle

    return EXIT_SUCCESS;
}

/*  www.beej.us
 ** server.c -- a stream socket server demo
 */

#define PORT "8080"

#define BACKLOG 10	 // how many pending connections queue will hold

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char** argv)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return EXIT_FAILURE;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) <0){
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) <0){
            perror("setsockopt");
            return EXIT_FAILURE;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) <0){
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return EXIT_FAILURE;
    }

    freeaddrinfo(servinfo); // all done with this structure
    
    if (listen(sockfd, BACKLOG) <0) {
        perror("listen");
        return EXIT_FAILURE;
    }


    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) <0) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    
    printf("server: waiting for connections at port %s...\n", PORT);
    
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd < 0) {
                perror("accept");
                continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            client_worker(new_fd);
            close(new_fd);
            return 0;
        }
        close(new_fd);  // parent doesn't need this
    }
    
    return (EXIT_SUCCESS);
}

