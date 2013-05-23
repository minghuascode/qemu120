#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#define UPDATE_MAGIC	0x46414B52	/* "RKAF" */

struct update_part {
        char name[32];			/* 0x00 */
        char path[64];			/* 0x20 */
        unsigned pos;			/* 0x60 */
        unsigned flashpos;		/* 0x64 */
        unsigned padded_size;		/* 0x68 */
        unsigned size;			/* 0x6c */
};

struct update_header {
        unsigned magic;			/* 0x00 */
        unsigned length;		/* 0x04 */
        char model[0x40];		/* 0x08 */
        char manufacturer[0x3c];	/* 0x48 */
	unsigned version;		/* 0x84 */
	unsigned num_parts;		/* 0x88 */

	struct update_part parts[16];	/* 0x8C */
};

unsigned ram_size, base_addr, atag_addr, krnl_addr;
char mtd_id[256];
int machine_id;
struct update_header out;

#define BOOTLOADER_MAGIC	"RK28@Copyright2008Rockchip"

struct bootloader_header {
	char magic[0x20];
	char reserved[0x10];
	unsigned short build_year;
	unsigned short build_month;
	unsigned short build_day;
	unsigned short build_hour;
	unsigned short build_minute;
	unsigned short build_second;
	unsigned int major;
	unsigned int minor;
	/* 104 (0x68) bytes */
};

static unsigned int crc_table[256] = {
	0x00000000, 0x04c10db7, 0x09821b6e, 0x0d4316d9,
	0x130436dc, 0x17c53b6b, 0x1a862db2, 0x1e472005,
	0x26086db8, 0x22c9600f, 0x2f8a76d6, 0x2b4b7b61,
	0x350c5b64, 0x31cd56d3, 0x3c8e400a, 0x384f4dbd,
	0x4c10db70, 0x48d1d6c7, 0x4592c01e, 0x4153cda9,
	0x5f14edac, 0x5bd5e01b, 0x5696f6c2, 0x5257fb75,
	0x6a18b6c8, 0x6ed9bb7f, 0x639aada6, 0x675ba011,
	0x791c8014, 0x7ddd8da3, 0x709e9b7a, 0x745f96cd,
	0x9821b6e0, 0x9ce0bb57, 0x91a3ad8e, 0x9562a039,
	0x8b25803c, 0x8fe48d8b, 0x82a79b52, 0x866696e5,
	0xbe29db58, 0xbae8d6ef, 0xb7abc036, 0xb36acd81,
	0xad2ded84, 0xa9ece033, 0xa4aff6ea, 0xa06efb5d,
	0xd4316d90, 0xd0f06027, 0xddb376fe, 0xd9727b49,
	0xc7355b4c, 0xc3f456fb, 0xceb74022, 0xca764d95,
	0xf2390028, 0xf6f80d9f, 0xfbbb1b46, 0xff7a16f1,
	0xe13d36f4, 0xe5fc3b43, 0xe8bf2d9a, 0xec7e202d,
	0x34826077, 0x30436dc0, 0x3d007b19, 0x39c176ae,
	0x278656ab, 0x23475b1c, 0x2e044dc5, 0x2ac54072,
	0x128a0dcf, 0x164b0078, 0x1b0816a1, 0x1fc91b16,
	0x018e3b13, 0x054f36a4, 0x080c207d, 0x0ccd2dca,
	0x7892bb07, 0x7c53b6b0, 0x7110a069, 0x75d1adde,
	0x6b968ddb, 0x6f57806c, 0x621496b5, 0x66d59b02,
	0x5e9ad6bf, 0x5a5bdb08, 0x5718cdd1, 0x53d9c066,
	0x4d9ee063, 0x495fedd4, 0x441cfb0d, 0x40ddf6ba,
	0xaca3d697, 0xa862db20, 0xa521cdf9, 0xa1e0c04e,
	0xbfa7e04b, 0xbb66edfc, 0xb625fb25, 0xb2e4f692,
	0x8aabbb2f, 0x8e6ab698, 0x8329a041, 0x87e8adf6,
	0x99af8df3, 0x9d6e8044, 0x902d969d, 0x94ec9b2a,
	0xe0b30de7, 0xe4720050, 0xe9311689, 0xedf01b3e,
	0xf3b73b3b, 0xf776368c, 0xfa352055, 0xfef42de2,
	0xc6bb605f, 0xc27a6de8, 0xcf397b31, 0xcbf87686,
	0xd5bf5683, 0xd17e5b34, 0xdc3d4ded, 0xd8fc405a,
	0x6904c0ee, 0x6dc5cd59, 0x6086db80, 0x6447d637,
	0x7a00f632, 0x7ec1fb85, 0x7382ed5c, 0x7743e0eb,
	0x4f0cad56, 0x4bcda0e1, 0x468eb638, 0x424fbb8f,
	0x5c089b8a, 0x58c9963d, 0x558a80e4, 0x514b8d53,
	0x25141b9e, 0x21d51629, 0x2c9600f0, 0x28570d47,
	0x36102d42, 0x32d120f5, 0x3f92362c, 0x3b533b9b,
	0x031c7626, 0x07dd7b91, 0x0a9e6d48, 0x0e5f60ff,
	0x101840fa, 0x14d94d4d, 0x199a5b94, 0x1d5b5623,
	0xf125760e, 0xf5e47bb9, 0xf8a76d60, 0xfc6660d7,
	0xe22140d2, 0xe6e04d65, 0xeba35bbc, 0xef62560b,
	0xd72d1bb6, 0xd3ec1601, 0xdeaf00d8, 0xda6e0d6f,
	0xc4292d6a, 0xc0e820dd, 0xcdab3604, 0xc96a3bb3,
	0xbd35ad7e, 0xb9f4a0c9, 0xb4b7b610, 0xb076bba7,
	0xae319ba2, 0xaaf09615, 0xa7b380cc, 0xa3728d7b,
	0x9b3dc0c6, 0x9ffccd71, 0x92bfdba8, 0x967ed61f,
	0x8839f61a, 0x8cf8fbad, 0x81bbed74, 0x857ae0c3,
	0x5d86a099, 0x5947ad2e, 0x5404bbf7, 0x50c5b640,
	0x4e829645, 0x4a439bf2, 0x47008d2b, 0x43c1809c,
	0x7b8ecd21, 0x7f4fc096, 0x720cd64f, 0x76cddbf8,
	0x688afbfd, 0x6c4bf64a, 0x6108e093, 0x65c9ed24,
	0x11967be9, 0x1557765e, 0x18146087, 0x1cd56d30,
	0x02924d35, 0x06534082, 0x0b10565b, 0x0fd15bec,
	0x379e1651, 0x335f1be6, 0x3e1c0d3f, 0x3add0088,
	0x249a208d, 0x205b2d3a, 0x2d183be3, 0x29d93654,
	0xc5a71679, 0xc1661bce, 0xcc250d17, 0xc8e400a0,
	0xd6a320a5, 0xd2622d12, 0xdf213bcb, 0xdbe0367c,
	0xe3af7bc1, 0xe76e7676, 0xea2d60af, 0xeeec6d18,
	0xf0ab4d1d, 0xf46a40aa, 0xf9295673, 0xfde85bc4,
	0x89b7cd09, 0x8d76c0be, 0x8035d667, 0x84f4dbd0,
	0x9ab3fbd5, 0x9e72f662, 0x9331e0bb, 0x97f0ed0c,
	0xafbfa0b1, 0xab7ead06, 0xa63dbbdf, 0xa2fcb668,
	0xbcbb966d, 0xb87a9bda, 0xb5398d03, 0xb1f880b4,
};

int
find_update_index(struct update_header *header, const char *partname)
{
	unsigned i;

	for (i=0; i < header->num_parts; i++)
		if (strcmp(header->parts[i].name, partname) == 0)
			return i;

	return -1;
}

void
show_bootloader(FILE *fp, struct update_part *part)
{
	struct bootloader_header header;

	fseek(fp, part->pos, SEEK_SET);
	fread(&header, sizeof(header), 1, fp);

	if (strcmp(header.magic, BOOTLOADER_MAGIC) == 0) {
		printf("Bootloader: v%d.%02d %d-%d-%d %d:%d:%d\n",
			header.major, header.minor,
			header.build_year, header.build_month, header.build_day,
			header.build_hour, header.build_minute, header.build_second);
	} else {
		printf("Invalid bootloader\n");
	}
}


int
show_kernel(FILE *fp, struct update_part *part)
{
	char *p, *buf = malloc(part->size);
	if (buf == NULL) {
		printf("Can't alloc memory\n");
		return -1;
	}

	fseek(fp, part->pos, SEEK_SET);
	if (fread(buf, part->size, 1, fp) != 1)
		goto fail;

	for (p = buf; p < buf + part->size; p++) {
		if (*p == 'L' && strncmp(p, "Linux version ", 14) == 0) {
			printf("Kernel: %s", p);
			free(buf);
			return 0;
		}
	}

fail:
	free(buf);
	return -1;
}


void
show_image(FILE *fp, struct update_part *part)
{
	char buf[0x44];

	fseek(fp, part->pos, SEEK_SET);
	fread(buf, sizeof(buf), 1, fp);
	buf[0x30+15] = '\0';
	printf("%s\n", buf+0x30);
}

void
show_manuf_and_model(struct update_header *header)
{
	printf(	"Manufacturer: %s\n"
		"Machine model: %s\n",
		header->manufacturer, header->model);
}

int
check_crc(const char *fname)
{
	unsigned int crc = 0, stored_crc;
	FILE *fp = fopen(fname, "rb");
	off_t size;
	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (size > 4) {
		char ch;
		int i;

		for (i=0; i < size - 4; i++) {
			fread(&ch, sizeof(ch), 1, fp);
			crc = (crc << 8) ^ crc_table[((crc >> 24) ^ ch) & 0xff];
		}
	}

	fread(&stored_crc, sizeof(stored_crc), 1, fp);
	fclose(fp);

	return stored_crc == crc ? 0 : -1;
}

int
check_update(const char *fname, struct update_header *header)
{
	FILE *fp;

	printf("Check file... ");
	fflush(stdout);

	if (check_crc(fname) != 0) {
		printf("Failed\n");
		return -1;
	}

	printf("OK\n");

	if ((fp=fopen(fname, "rb")) == NULL) {
		printf("Can't open file: %s\n", fname);
		return -1;
	}

	if (fread(header, sizeof(*header), 1, fp) != 1) {
		printf("read failed\n");
		return -1;
	}

	if (header->magic != UPDATE_MAGIC) {
		printf("Invalid image file\n");
		return -1;
	}

	return 0;
}

int
show_update_info(const char *fname)
{
	struct update_header header;
	FILE *fp = fopen(fname, "rb");
	int idx;

	if (check_update(fname,&header))
		return -1;

	show_manuf_and_model(&header);
	fp = fopen(fname, "rb");

	if ((idx=find_update_index(&header, "bootloader")) >= 0)
		show_bootloader(fp, &header.parts[idx]);

	if ((idx=find_update_index(&header, "kernel")) >= 0)
		show_kernel(fp, &header.parts[idx]);

	if ((idx=find_update_index(&header, "boot")) >= 0) {
		printf("Boot: ");
		show_image(fp, &header.parts[idx]);
	}

	if ((idx=find_update_index(&header, "recovery")) >= 0) {
		printf("Recovery: ");
		show_image(fp, &header.parts[idx]);
	}

	if ((idx=find_update_index(&header, "system")) >= 0) {
		printf("System: ");
		show_image(fp, &header.parts[idx]);
	}

	return 0;
}

int
parse_partitions(char parms[50][200], int count)
{
	int i;

	mtd_id[0] = 0;
	out.num_parts = 0;

	for (i = 1; i < count; i++) {
		if (sscanf(parms[i], "mtd_id=%s", mtd_id) >= 1)
			continue;
#if 0
		if (sscanf(parms[i], "%s 0x%x:0x%x:%s", ) == 4 ||
			sscanf(parms[i], "%s 0x%x:0x%x", ) == 3) {
			++out.num_parts;
		}
#endif
	}

	return 0;
}

int
parse_machineinfo(char parms[50][200], int count)
{
	int i;

	for (i = 1; i < count; i++) {
		if (sscanf(parms[i], "manufacturer=%s", out.manufacturer) == 1)
			continue;
		if (sscanf(parms[i], "machine_model=%s", out.model) == 1)
			continue;
		if (sscanf(parms[i], "magic=0x%x", &out.magic) == 1)
			continue;
		if (sscanf(parms[i], "machine_id=%d", &machine_id) == 1)
			continue;
	}

	return 0;
}

int
parse_ram(char parms[50][200], int count)
{
	int i;

	ram_size =
	base_addr =
	atag_addr =
	krnl_addr = -1;

	for (i = 1; i < count; i++) {
		if (sscanf(parms[i], "size=%d", &ram_size) == 1)
			continue;
		if (sscanf(parms[i], "base_addr=0x%x", &base_addr) == 1)
			continue;
		if (sscanf(parms[i], "atag_addr=0x%x", &atag_addr) == 1)
			continue;
		if (sscanf(parms[i], "krnl_addr=0x%x", &krnl_addr) == 1)
			continue;
	}

	return 0;
}

int
parse_section(char parms[50][200], int count)
{
	char buf[200], *p;

	strcpy(buf, parms[0]+strlen("BEGIN"));
	p = buf;

	while(*p == ' ')
		++p;

	if (strcmp(p, "RAM") == 0)
		parse_ram(parms, count);
	else if (strcmp(p, "PARTITIONS") == 0)
		parse_partitions(parms, count);
	else if (strcmp(p, "MACHINE_INFO") == 0)
		parse_machineinfo(parms, count);
	else
		printf("Unknown TAG: %s\n", p);

	return 0;
}

int
parse_hwdef(const char *fname)
{
	char params[50][200];
	char line[200], *p;
	int param_count = 0;
	FILE *fp;

	if ((fp=fopen(fname,"r")) == NULL) {
		printf("Can't open file: %s\n", fname);
		return -1;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';
		if (line[strlen(line)-1] == '\r')
			line[strlen(line)-1] = '\0';

		p = line;
		while(*p == ' ')
			++p;

		while(p[strlen(p)-1] == ' ')
			p[strlen(p)-1] = '\0';

		while(*p == '\t')
			++p;

		while(p[strlen(p)-1] == '\t')
			p[strlen(p)-1] == '\0';

		if (*p == '#')
			continue;

		if (param_count) {
			strcpy(params[param_count++], p);

			if (strncmp(p, "END", 3) == 0) {
				parse_section(params, param_count);
				param_count = 0;
			}
		} else {
			if (strncmp(p, "BEGIN", 5) == 0) {
				strcpy(params[0], p);
				param_count = 1;
			}
		}
	}

	if (!feof(fp)) {
		printf("File read failed!\n");
		fclose(fp);
		return -3;
	}

	return 0;
}

int
pack_update(int argc, char **argv)
{
	char buf[256];

	printf("------ PACKAGE ------\n");

	snprintf(buf, sizeof(buf), "%s/%s", argv[2], "HWDEF");
	if (parse_hwdef(buf))
		return -1;

	return 0;
}

int
extract_file(FILE *ifp, const char *outdir, struct update_part *part)
{
	unsigned int left = part->size;
	char outbuf[0x800];
	char *p, *c;
	FILE *ofp;

	snprintf(outbuf, sizeof(outbuf), "%s/%s", outdir, part->path);
	c=outbuf;

	while ((p=strchr(c,'/')) != NULL || (p=strrchr(c, '\\')) != NULL) {
		*p = '\0';
		if (mkdir(outbuf, 0755) != 0 && errno != EEXIST) {
			printf("Can't create directory: %s\n", outbuf);
			return -1;
		}

		*p = '/';
		c = p + 1;		
	}

	if ((ofp=fopen(outbuf, "wb")) == NULL) {
		printf("Can't open/create file: %s\n", outbuf);
		return -1;
	}

	fseek(ifp, part->pos, SEEK_SET);

	if (strcmp(part->name, "parameter") == 0)
		fseek(ifp, 8, SEEK_CUR);

	while(left) {
		unsigned int toread = (left >= 0x800) ? 0x800 : left;
		fread(outbuf, toread, 1, ifp);
		fwrite(outbuf, toread, 1, ofp);
		left -= toread;
	}

	fclose(ofp);

	return 0;
}

int
unpack_update(int argc, char **argv)
{
	struct update_header header;
	FILE *ifp;

	if (check_update(argv[2], &header))
		return -1;
 
	ifp = fopen(argv[2], "rb");
	fseek(ifp, 2048, SEEK_SET);

	printf("------- UNPACK -------\n");
	if (header.num_parts) {
		unsigned i;
		for (i=0; i < header.num_parts; i++) {
			printf("%s\t0x%08X\t0x%08X\n", header.parts[i].path, header.parts[i].pos, header.parts[i].size);

			if (strcmp(header.parts[i].path, "SELF") == 0) {
				printf("Skip SELF file.\n");
				continue;
			}

			extract_file(ifp, argv[3], header.parts + i);
		}
	}

	return 0;
}

void
usage(const char *appname)
{
	const char *p = strrchr(appname,'/');
	p = p ? p + 1 : appname;

	printf(	"USAGE:\n"
		"\t%s {Option} Src [Dest]\n"
		"Example:\n"
		"\t%s -info update.img\tShow file info\n"
		"\t%s -pack xxx update.img\tPack files\n"
		"\t%s -unpack update.img xxx\tunpack files\n",
		p, p, p, p);
}

int
main(int argc, char** argv)
{
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "-pack") == 0 && argc >= 3) {
		if (pack_update(argc,argv) == 0)
			printf("Pack OK!\n");
		else
			printf("Pack failed\n");
	} else if (strcmp(argv[1], "-unpack") == 0 && argc >= 3) {
		if (unpack_update(argc,argv) == 0)
			printf("Unpack OK!\n");
		else
			printf("Unpack failed\n");
	} else if (strcmp(argv[1], "-info") == 0)
		show_update_info(argv[2]);
	else
		usage(argv[0]);

	return 0;
}
