#
# Makefile
#

.phony: all clean
	
all: clean
	$(CROSS_COMPILE)gcc -g -c -I . -o base64.o base64_enc.c
	$(CROSS_COMPILE)gcc -g -c -I . -o sha1.o   sha1.c
	$(CROSS_COMPILE)gcc -g -c -I . -o webs.o   websocket.c
	$(CROSS_COMPILE)gcc -g -c -I . -o wssmain.o  wssmain.c
	$(CROSS_COMPILE)gcc -g -c -I . -o test.o     test.c
	$(CROSS_COMPILE)gcc -g test.o wssmain.o base64.o sha1.o webs.o
	echo "Built *.o a.out"

clean:
	rm *.o a.out
	echo "Cleaned *.o a.out"

