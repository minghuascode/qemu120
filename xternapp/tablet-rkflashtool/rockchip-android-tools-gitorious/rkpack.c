/*-
 * Copyright (c) 2010 FUKAUMI Naoki.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	uint8_t *buf;

	if ((buf = malloc(2 * 1024)) == NULL)
		err(EXIT_FAILURE, "malloc");

	memset(buf, 0, 2 * 1024);

	buf[0] = 'R';
	buf[1] = 'K';
	buf[2] = 'A';
	buf[3] = 'F';

	strncpy(&buf[0x08], argv[1], 0x40);
	strncpy(&buf[0x48], argv[2], 0x40);

	argc -= 3;

	buf[0x88] = (argc >> 0) & 0xff;
	buf[0x89] = (argc >> 8) & 0xff;
	buf[0x8a] = (argc >> 16) & 0xff;
	buf[0x8b] = (argc >> 32) & 0xff;

	free(buf);

	return EXIT_SUCCESS;
}
