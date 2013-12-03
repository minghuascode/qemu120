/*
 * tcrypt64.c
 * 64-bit crypt and decrypt
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define __USE_GNU
#include <crypt.h>

struct crypt_data workspace;
const char key[] = { /* 64 bytes */
    1,0,0,1, 1,1,0,0,  1,1,0,1, 1,1,1,0, 
    0,0,0,1, 0,1,0,0,  1,1,1,1, 0,1,1,0, 
    1,1,0,1, 1,1,1,0,  1,0,0,1, 1,0,1,0, 
    1,1,1,1, 1,0,0,1,  0,1,0,0, 1,0,1,1
};

int buflen = 0;
char buf[128000];

int main(int argc, char *argv[])
{
    char *fname = NULL;
    char cmd = '\0';
    int ifd = 0;
    int len = 0;
    int edflag = 0; /* 0 for encoding */
    int i = 0;
    
    if ( argc != 3 ) {
        printf("\nError: argc %d must 3\n", argc);
        return 1;
    }
    
    memset(&workspace, 0, sizeof(workspace));
    setkey_r(key, &workspace);
    
    cmd = argv[1][0];
    fname = argv[2];
    
    ifd = open(fname, O_RDONLY);
    if ( ifd <= 0 ) {
        printf("\nError: open file %s\n", fname);
        return 1;
    }

    memset(buf, 0, sizeof(buf));    
    len = read(ifd, buf, sizeof(buf));
    close(ifd);
    if ( len <= 0 ) {
        printf("\nError: read file %s\n", fname);
        close(ifd);
        return 1;
    }
    buflen = len;
    
    if ( cmd == 'c' ) {
        edflag = 0; /* encoding */
        printf("Encoding %d\n", buflen);
        for ( i=0; i<buflen; i+=8 ) {
            char blk[64];
            int j=0;
            memset(blk, 0, sizeof(blk));
            for ( j=0; j<8; j++) {
                char b=1;
                int k=0;
                for (k=0; k<8; k++) {
                    char x = (b<<k);
                    //printf("\n x=%02x buf[%d]=%02x ", x, i+j, buf[i+j]);
                    if ( (x & buf[i+j]) != 0 ) {
                        int m = j*8+k;
                        blk[m] = 1;
                        //printf("\n blk[%d] = 1", m);
                    }
                }
            }
            //printf("\nEncoding\n");
            //for ( j=0; j<64; j++ ) { printf("%d", blk[j]); }
            //printf("\n");
            encrypt_r(blk, edflag, &workspace);
            for ( j=0; j<64; j++ ) { 
                printf("%d", blk[j]); 
            }
        }
    } else if ( cmd == 'c' ) {
        edflag = 1; /* decoding */
        printf("Decoding %d\n", buflen);
        for ( i=0; i<buflen; i+=64 ) {
            char blk[64];
            int j=0;
            memset(blk, 0, sizeof(blk));
            for ( j=0; j<64; j++ ) {
                blk[j] = (buf[i+j]=='1'?1:0);
            }
            //printf("\nDecoding\n");
            //for ( j=0; j<64; j++ ) { printf("%c", buf[i+j]); }
            //printf("\n");
            //for ( j=0; j<64; j++ ) { printf("%d", blk[j]); }
            //printf("\n");
            encrypt_r(blk, edflag, &workspace);
            //printf("\nDecoded\n");
            //for ( j=0; j<64; j++ ) { printf("%d", blk[j]); }
            //printf("\n");
            for ( j=0; j<8; j++) {
                char d = 0;
                char b=1;
                int k=0;
                for (k=0; k<8; k++) {
                    if ( blk[j*8+k] == 1 ) {
                        d |= (b<<k);
                    } else if ( blk[j*8+k] == 0 ) {
                        ;
                    } else {
                        printf("\nError at i %d j %d k %d\n", i, j, k);
                    }
                }
                printf("%c", d);
            }
        }
    } else {
        printf("\nError: cmd must be either c or d. %s\n", cmd);
        return 1;
    }
    return 0;
}

