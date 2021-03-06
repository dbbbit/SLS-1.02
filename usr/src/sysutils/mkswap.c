/*
 * mkswap.c - set up a linux swap device
 *
 * Copyright 1991 Linus Torvalds.
 * This file may be redistributed as per the Linux copyright.
 */

/*
 * 20.12.91  -	time began. Got VM working yesterday by doing this by hand.
 *
 * Usuage: mkswap [-c] device size-in-blocks
 *
 *	-c for readablility checking (use it unless you are SURE!)
 *
 * The device may be a block device or a image of one, but this isn't
 * enforced (but it's not much fun on a character device :-). 
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#undef malloc
#define malloc Malloc
#include <linux/mm.h>
#undef malloc

#ifndef __GNUC__
#error "needs gcc for the bitop-__asm__'s"
#endif

#ifndef __linux__
#define volatile
#endif

#define TEST_BUFFER_PAGES 8

static char *program_name = "mkswap";
static char *device_name = NULL;
static int DEV = -1;
static long PAGES = 0;
static int check = 0;
static int badpages = 0;

static char signature_page[PAGE_SIZE];

#define bitop(name,op) \
static inline int name(char * addr,unsigned int nr) \
{ \
int __res; \
__asm__ __volatile__("bt" op " %1,%2; adcl $0,%0" \
:"=g" (__res) \
:"r" (nr),"m" (*(addr)),"0" (0)); \
return __res; \
}

bitop(bit,"")
bitop(setbit,"s")
bitop(clrbit,"r")

/*
 * Volatile to let gcc know that this doesn't return. When trying
 * to compile this under minix, volatile gives a warning, as
 * exit() isn't defined as volatile under minix.
 */
volatile void fatal_error(const char * fmt_string) {

	fprintf(stderr,fmt_string,program_name,device_name);
	exit(1);
}

#define usage() fatal_error("usage: %s [-c] /dev/name blocks\n")
#define die(str) fatal_error("%s: " str "\n")

void check_blocks(void) {

	unsigned int current_page;
	int do_seek = 1;
	static char buffer[PAGE_SIZE];

	current_page = 0;
	while (current_page < PAGES) {
		if (!check) {
			setbit(signature_page,current_page++);
			continue;
		}
		if (do_seek && lseek(DEV,current_page*PAGE_SIZE,SEEK_SET) !=
		current_page*PAGE_SIZE)
			die("seek failed in check_blocks.");
		if (do_seek = (PAGE_SIZE != read(DEV, buffer, PAGE_SIZE))) {
			clrbit(signature_page,current_page++);
			badpages++;
			continue;
		}
		setbit(signature_page,current_page++);
	}
	if (badpages)
		printf("%d bad page%s\n",badpages,(badpages>1)?"s":"");
}

int main(int argc, char **argv) {
	char *tmp;
	struct stat statbuf;
	int goodpages;

	memset(signature_page,0,PAGE_SIZE);
	if (argc && argv[0])
		program_name = argv[0];
	while (argc-- > 1) {
		argv++;
		if (argv[0][0] != '-')
			if (device_name) {
				PAGES = strtol(argv[0],&tmp,0)>>2;
				if (*tmp)
					usage();
			} else
				device_name = argv[0];
		else while (*++argv[0])
			switch (argv[0][0]) {
				case 'c': check=1; break;
				default: usage();
			}
	}
	if (!device_name || PAGES<10 || PAGES > 32768)
		usage();
	DEV = open(device_name,O_RDWR);
	if (DEV<0)
		die("unable to open %s.");
	if (fstat(DEV,&statbuf)<0)
		die("unable to stat %s.");
	if (!S_ISBLK(statbuf.st_mode))
		check=0;
	else if (statbuf.st_rdev == 0x0300 || statbuf.st_rdev == 0x0340)
		die("will not try to make swapdevice on '%s'.");
	check_blocks();
	if (!clrbit(signature_page,0))
		die("fatal: first page unreadable.");
	goodpages = PAGES - badpages - 1;
	if (!goodpages)
		die("unable to set up swap-space: unreadable.");
	printf("Setting up swapspace, size = %d bytes\n",goodpages*PAGE_SIZE);
	strncpy(signature_page+PAGE_SIZE-10,"SWAP-SPACE",10);
	if (lseek(DEV, 0, SEEK_SET))
		die("unable to rewind swap-device.");
	if (PAGE_SIZE != write(DEV, signature_page, PAGE_SIZE))
		die("unable to write signature page/");

	sync();
	return 0;
}
