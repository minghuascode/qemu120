/*
 * Copyright (c) 2013 Putilov Andrey
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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "websocket.h"

#define PORT 8088
#define BUF_LEN 0xFFFF
#define PACKET_DUMP

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void showpkt(const uint8_t *buffer, size_t bufferSize)
{
    unsigned int i=0;
    if ( buffer == NULL ) return;
    for (i=0; i<bufferSize; i+= 16) {
      unsigned int j=0;
      for (j=0; j<16 && i+j<bufferSize; j++) {
	if ( (j%4) == 0 ) printf(" ");
	printf(" %02x", buffer[i+j]);
      }
      printf(" ");
      for (j=0; j<16 && i+j<bufferSize; j++) {
	uint8_t c = buffer[i+j];
	if ( (j%4) == 0 ) printf(" ");
	if ( c >= ' ' && c <= 126 ) printf("%c",c);
	else printf(".");
      }
      printf("\n");
    }
    /*was: fwrite(buffer, 1, bufferSize, stdout);*/
}

#include <sys/time.h>
#include <errno.h>
/* static: */
struct timeval;
static int64_t calcTmDiff(struct timeval * t1, struct timeval * t2)
{
    int64_t tm1 = 0, tm2 = 0;
    tm1 = t1->tv_sec; tm1 *= 1000000; tm1 += t1->tv_usec;
    tm2 = t2->tv_sec; tm2 *= 1000000; tm2 += t2->tv_usec;
    int64_t tdif = tm2 - tm1;
    if ( tdif < 0 ) { 
        return -1;
    } else if ( tdif < 0x7ffffffeUL ) {
        return (uint32_t)(tdif & 0x7fffffffUL);
    }
    return 0x7ffffffe;
}

static uint32_t calcMsTm(struct timeval * t1)
{
    int64_t tm1 = 0;
    tm1 = t1->tv_sec; tm1 *= 1000000; tm1 += t1->tv_usec;
    tm1 /= 1000;
    return (uint32_t)(tm1 & 0xffffffffUL);
}

static uint32_t calcMsTmDiff(uint32_t t1, uint32_t t2)
{
    return t2 - t1;
}

#define TIME_STAMP(x) struct timeval x = {0}; gettimeofday(&x, NULL)

static const char * decodeFrameType(enum wsFrameType ftyp)
{
    char * retp = NULL;
    switch (ftyp) {
    case WS_EMPTY_FRAME: /*= 0xF0,*/ retp = "empty_frame"; break;
    case WS_ERROR_FRAME: /*= 0xF1,*/ retp = "error_frame"; break;
    case WS_INCOMPLETE_FRAME: /* = 0xF2,*/ retp = "incomplete_frame"; break;
    case WS_TEXT_FRAME:  /* = 0x01,*/ retp = "text_frame"; break;
    case WS_BINARY_FRAME: /* = 0x02,*/ retp = "binary_frame"; break;
    case WS_PING_FRAME:   /* = 0x09,*/ retp = "ping_frame"; break;
    case WS_PONG_FRAME:   /* = 0x0A,*/ retp = "pong_frame"; break;
    case WS_OPENING_FRAME: /* = 0xF3,*/ retp = "opening_frame"; break;
    case WS_CLOSING_FRAME: /* = 0x08*/ retp = "closing_frame"; break;
    default: retp = "unknown_frame"; break;
    }
    return retp;
}

#include <sys/types.h> /* for open */
#include <sys/stat.h>  /* for open */
#include <fcntl.h>     /* for open */

int safeSend(int clientSocket, const uint8_t *buffer, size_t bufferSize)
{
    #ifdef PACKET_DUMP
    printf("out packet:\n");
    showpkt(buffer, bufferSize);
    printf("\n");
    #endif
    ssize_t written = send(clientSocket, buffer, bufferSize, 0);
    if (written == -1) {
        close(clientSocket);
        perror("send failed");
        return EXIT_FAILURE;
    }
    if (written != bufferSize) {
        close(clientSocket);
        perror("written not all bytes");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

void clientWorker(int clientSocket)
{
    uint8_t buffer[BUF_LEN];
    memset(buffer, 0, BUF_LEN);
    size_t readedLength = 0;
    size_t frameSize = BUF_LEN;
    enum wsState state = WS_STATE_OPENING;
    uint8_t *data = NULL;
    size_t dataSize = 0;
    enum wsFrameType frameType = WS_INCOMPLETE_FRAME;
    struct handshake hs;
    nullHandshake(&hs);
    
    #define prepareBuffer frameSize = BUF_LEN; memset(buffer, 0, BUF_LEN);
    #define initNewFrame frameType = WS_INCOMPLETE_FRAME; readedLength = 0; memset(buffer, 0, BUF_LEN);
    
    TIME_STAMP(tm00);
    while (frameType == WS_INCOMPLETE_FRAME) {
        
        /* retry and receive till timeout */
        ssize_t readed = 0;
        int retries = 0;
        TIME_STAMP(tm0);
        for (retries = 0; retries < 1200; retries ++) {
            TIME_STAMP(tm1);
            if ( calcTmDiff(&tm0, &tm1) > 10000000000LL ) {
                printf(" time cost %lld\n", calcTmDiff(&tm0, &tm1));
                perror("recv idle timeout");
                readed = 0; 
                break;
            }
            readed = recv(clientSocket, buffer+readedLength, 
                                            BUF_LEN-readedLength, MSG_DONTWAIT);
            if ( readed < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                usleep(1000);
                readed = 0; 
                continue;
            }
            if (readed == 0) {
                perror("recv closed");
                readed = 0; 
                break;
            }
            if (readed < 0) {
                perror("recv failed");
                readed = 0; 
                break;
            }
            TIME_STAMP(tm2);
            printf("recv %d bytes in %u ms with %d retries\n", 
                readed, calcMsTmDiff(calcMsTm(&tm0), calcMsTm(&tm2)), retries);
            break;
        }
        if ( retries == 200 ) {
            TIME_STAMP(tm2);
            printf("recv %d bytes in %u ms with %d retries\n", 
                readed, calcMsTmDiff(calcMsTm(&tm0), calcMsTm(&tm2)), retries);
        }
        if (readed == 0) {
            break;
        }
        
        #ifdef PACKET_DUMP
        if (state == WS_STATE_OPENING) {
          printf("in packet state opening:\n");
          fwrite(buffer, 1, readed, stdout);
          printf("\n");
        }
        printf("in packet:\n");
        showpkt(buffer, readed);
        printf("\n");
        #endif
        readedLength+= readed;
        assert(readedLength <= BUF_LEN);

        if (state == WS_STATE_OPENING) {
            frameType = wsParseHandshake(buffer, readedLength, &hs);
        } else {
            frameType = wsParseInputFrame(buffer, readedLength, &data, &dataSize);
        }
#if (1) /* special handling to feed the web page */
        printf("rx frameType 0x%x %s\n", frameType, decodeFrameType(frameType));
        if ( frameType == WS_OPENING_FRAME ) {
            if ( strcmp(hs.resource, "/wstest"     ) == 0 || 
                 strcmp(hs.resource, "/wstest.htm" ) == 0 || 
                 strcmp(hs.resource, "/wstest.html") == 0    ) {
                static unsigned int pagecnt = 0;
                pagecnt ++;
                frameSize = sprintf((char *)buffer, 
                                    "HTTP/1.1 200 OK\r\n"
                                    /*"%s\r\n%s\r\n"*/
                                    "%s\r\n\r\n"
                                "%s %u %s\r\n\r\n", 
                            "Content-Type: text/html", 
                            /*"Pragma: no-cache", "Cache-Control: no-cache", */
                        "<html><body><p>good", pagecnt, "</p></body></html>");
                safeSend(clientSocket, buffer, frameSize);
                int fn = open("wstest.html", O_RDONLY);
                if ( fn <= 0 ) {
                    frameSize = sprintf((char *)buffer, 
                        "<html><body><p>no page file found</p></body></html>");
                    safeSend(clientSocket, buffer, frameSize);
                } else {
                    prepareBuffer;
                    int len = read(fn, buffer, frameSize);
                    if ( len > 0 ) {
                        safeSend(clientSocket, buffer, len);
                             
                        frameSize = sprintf((char *)buffer, 
                         "<iframe id=\"frame1\" name=\"frame1\" src=\"http://");
                        safeSend(clientSocket, buffer, frameSize);
                        
                        frameSize = sprintf((char *)buffer, "%s", hs.host);
                        safeSend(clientSocket, buffer, frameSize);
                        
                        frameSize = sprintf((char *)buffer, 
                                    "/wstestif.html\"></iframe>\r\n\r\n");
                        safeSend(clientSocket, buffer, frameSize);
                        
                        frameSize = sprintf((char *)buffer, 
                                "\r\n </div> \r\n</body>\r\n</html>\r\n\r\n");
                        safeSend(clientSocket, buffer, frameSize);
 
                    } else {
                        frameSize = sprintf((char *)buffer, 
                                "<html><body><p>page file read error"
                                "</p></body></html>");
                        safeSend(clientSocket, buffer, frameSize);
                    }
                    close(fn);
                }
                break;
            }
            if ( hs.resource ) {
              char * rsc = &hs.resource[1]; /* skip leading / */
              unsigned rlen = strlen(hs.resource);
              char * dot = strrchr(rsc, '.');
              if ( dot != NULL && 
                   (strcmp(dot, ".htm" ) == 0 || strcmp(dot, ".html") == 0) ) {
                frameSize = sprintf((char *)buffer, 
                        "HTTP/1.1 200 OK\r\n"
                        "%s\r\n%s\r\n%s\r\n\r\n", 
                        "Content-Type: text/html", 
                        "Pragma: no-cache", "Cache-Control: no-cache");
                safeSend(clientSocket, buffer, frameSize);
                int fn = open(rsc, O_RDONLY);
                if ( fn <= 0 ) {
                    frameSize = sprintf((char *)buffer, 
                        "<html><body><p>no page file found</p></body></html>");
                    safeSend(clientSocket, buffer, frameSize);
                } else {
                    int len = 0; 
                    prepareBuffer;
                    len = read(fn, buffer, frameSize);
                    while ( len > 0 ) {
                        safeSend(clientSocket, buffer, len);
                        prepareBuffer;
                        len = read(fn, buffer, frameSize);
                    }
                    close(fn);
                }
                break;
              }
              if ( dot != NULL && 
                   (strcmp(dot, ".js" ) == 0 ) ) {
                frameSize = sprintf((char *)buffer, 
                        "HTTP/1.1 200 OK\r\n"
                        "%s\r\n%s\r\n%s\r\n\r\n", 
                        "Content-Type: text/javascript", 
                        "Pragma: no-cache", "Cache-Control: no-cache");
                safeSend(clientSocket, buffer, frameSize);
                int fn = open(rsc, O_RDONLY);
                if ( fn <= 0 ) {
                    frameSize = sprintf((char *)buffer, 
                        "<html><body><p>no page file found</p></body></html>");
                    safeSend(clientSocket, buffer, frameSize);
                } else {
                    int len = 0; 
                    prepareBuffer;
                    len = read(fn, buffer, frameSize);
                    while ( len > 0 ) {
                        safeSend(clientSocket, buffer, len);
                        prepareBuffer;
                        len = read(fn, buffer, frameSize);
                    }
                    close(fn);
                }
                break;
              }
            }
            if (strcmp(hs.resource, "/echo") != 0) {
                frameSize = sprintf((char *)buffer, 
                                    "HTTP/1.1 404 Not Found\r\n\r\n");
                safeSend(clientSocket, buffer, frameSize);
                break;
            }
            if ( !(hs.key) ) {
                prepareBuffer;
                frameSize = sprintf((char *)buffer,
                                    "HTTP/1.1 400 Bad Request\r\n"
                                    "%s%s\r\n\r\n",
                                    versionField,
                                    version);
                safeSend(clientSocket, buffer, frameSize);
                break;
            }
        }
#endif
        
        if ((frameType == WS_INCOMPLETE_FRAME && readedLength == BUF_LEN) || frameType == WS_ERROR_FRAME) {
            if (frameType == WS_INCOMPLETE_FRAME)
                printf("buffer too small");
            else
                printf("error in incoming frame\n");
            
            if (state == WS_STATE_OPENING) {
                prepareBuffer;
                frameSize = sprintf((char *)buffer,
                                    "HTTP/1.1 400 Bad Request\r\n"
                                    "%s%s\r\n\r\n",
                                    versionField,
                                    version);
                safeSend(clientSocket, buffer, frameSize);
                break;
            } else {
                prepareBuffer;
                wsMakeFrame(NULL, 0, buffer, &frameSize, WS_CLOSING_FRAME);
                if (safeSend(clientSocket, buffer, frameSize) == EXIT_FAILURE)
                    break;
                state = WS_STATE_CLOSING;
                initNewFrame;
            }
        }
        
        if (state == WS_STATE_OPENING) {
            assert(frameType == WS_OPENING_FRAME);
            if (frameType == WS_OPENING_FRAME) {
                // if resource is right, generate answer handshake and send it
                if (strcmp(hs.resource, "/echo") != 0) {
                    frameSize = sprintf((char *)buffer, "HTTP/1.1 404 Not Found\r\n\r\n");
                    if (safeSend(clientSocket, buffer, frameSize) == EXIT_FAILURE)
                        break;
                }
            
                prepareBuffer;
                wsGetHandshakeAnswer(&hs, buffer, &frameSize);
                if (safeSend(clientSocket, buffer, frameSize) == EXIT_FAILURE)
                    break;
                state = WS_STATE_NORMAL;
                initNewFrame;
            }
        } else {
            if (frameType == WS_CLOSING_FRAME) {
                if (state == WS_STATE_CLOSING) {
                    break;
                } else {
                    prepareBuffer;
                    wsMakeFrame(NULL, 0, buffer, &frameSize, WS_CLOSING_FRAME);
                    safeSend(clientSocket, buffer, frameSize);
                    break;
                }
            } else if (frameType == WS_TEXT_FRAME) {
                uint8_t *recievedString = NULL;
                recievedString = malloc(dataSize+1);
                assert(recievedString);
                memcpy(recievedString, data, dataSize);
                recievedString[ dataSize ] = 0;
                #ifdef PACKET_DUMP
                    printf("in packet string:\n");
                    showpkt(recievedString, dataSize);
                    printf("\n");
                #endif
                
                prepareBuffer;
                wsMakeFrame(recievedString, dataSize, buffer, &frameSize, WS_TEXT_FRAME);
                if (safeSend(clientSocket, buffer, frameSize) == EXIT_FAILURE)
                    break;
                initNewFrame;
            }
        }
    } // read/write cycle
    TIME_STAMP(tm11);
    printf("client socket done in %lld ns\n", calcTmDiff(&tm00, &tm11));
    
    close(clientSocket);
}

/* signal handler and backlog define copied from beej.us */
#define BACKLOG 10	 // how many pending connections queue will hold

#include <signal.h>
void sigchld_handler(int s)
{
#if ! defined(__ARMEL__)
    while(waitpid(-1, NULL, WNOHANG) > 0);
#endif
}

int main(int argc, char** argv)
{
    int yes=1;
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        error("create socket failed");
    }
    
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int))<0){
        error("setsockopt failed");
    }
    
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(PORT);
    if (bind(listenSocket, (struct sockaddr *) &local, sizeof(local)) == -1) {
        error("bind failed");
    }
    
    if (listen(listenSocket, BACKLOG) == -1) {
        error("listen failed");
    }
    
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) <0) {
        error("sigaction");
    }
    
    printf("opened %s:%d\n", inet_ntoa(local.sin_addr), ntohs(local.sin_port));
    
    while (TRUE) {
        struct sockaddr_in remote;
        socklen_t sockaddrLen = sizeof(remote);
        int clientSocket = accept(listenSocket, (struct sockaddr*)&remote, 
                                                                &sockaddrLen);
        if (clientSocket == -1) {
            error("accept failed");
        }
        
        printf("connected %s:%d\n", inet_ntoa(remote.sin_addr), 
                                                        ntohs(remote.sin_port));
        clientWorker(clientSocket);
        printf("disconnected\n");
    }
    
    close(listenSocket);
    return EXIT_SUCCESS;
}

