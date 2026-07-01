/* $Id: dcclog.c,v 1.10 2006/09/28 19:31:45 bob Exp $ 
 *
 * File:	dcclog.c
 * Module:
 * Project:	ARM-DCC logger expander
 * Author:	Robinson Mittmann (bob@boreste.com, bob@methafora.com.br)
 * Target:
 * Comment:
 * Copyright(C) 2005 Robinson Mittmann. All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#if defined(WIN32)
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
  #ifndef in_addr_t
    #define in_addr_t uint32_t
  #endif
#else
  #include <pthread.h>
  #include <assert.h>
#endif

#include <stdbool.h>

#include "arm_elf.h"
#include "log.h"

#include "debug.h"

#define VERSION_MAJOR 5
#define VERSION_MINOR 0

char * prog;
int verbose = 0;

int hexdump_g1(FILE * f, uint32_t  addr, const void * buf, int size);
int hexdump(FILE * f, unsigned int addr, void * ptr, unsigned int count);

void print_mem_blocks(bool dump)
{
	int i;

	printf(" - mem blocks: (%d)\n", mem_count);
	printf("       start:        end:     size:\n");
	for (i = 0; i < mem_count; i++) {
		printf("   0x%08x  0x%08x  %8d\n", mem[i].addr, 
			   mem[i].addr + mem[i].size - 1, mem[i].size);
		if (dump) {
			hexdump(stdout, mem[i].addr, mem[i].image, mem[i].size);
		}
	}
}

void print_assert_entries(void)
{
	int file_width = 0;
	int function_width = 0;
	struct log_def * log;
	int w;
	int i;

	printf(" - assert entries: (%d)\n", log_count);
	for (i = 0; i < log_count; i++) {
		log = &logtab[i];
		w = strlen(log->name);
		file_width = (file_width >= w) ? file_width : w; 
		w = strlen(log->function);
		function_width = (function_width >= w) ? function_width : w; 
	}

	printf("%8s %*s:%4s %*s: %s:\n", 
		   "addr", file_width, "file", "line", 
		   function_width, "function", "expression" ); 
	for (i = 0; i < log_count; i++) {
		log = &logtab[i];

		printf("%8x  %*s:%-4d  %*s '%s'\n",
			   log->addr, file_width, log->name, log->line, 
			   function_width, log->function, log->exp);
	}
}

void print_symbols(void)
{
	int i;

	printf(" - symbols: (%d)\n", sym_count);
	printf("        addr:    size:  name:\n");
	for (i = 0; i < sym_count; ++i) {
		printf("   0x%08x  %7d  %s\n", sym[i].addr, sym[i].size, sym[i].name);
	}
}

/*
 * Adjust the address offset and possibly endianness of
 * log entries
 * TODO: endianness
 */
int fix_log(void) 
{
	struct log_def * log;
	struct assert_entry * entry;
	char * file;
	char * dir;
	char * name;
	char * cp;
	int i;

	for (i = 0; i < log_count; i++) {
		log = &logtab[i];

		if ((entry = image_ptr(log->addr)) == NULL)
			return -1;

		if ((file = image_ptr(entry->file)) == NULL)
			return -2;

		name = file;
		dir = file;
		cp = file + strlen(file);
		while (cp > file) {
			char c = *(--cp);
			if ((c == '/') || (c == '\\')) {
				*cp = '\0';
				name = cp + 1;
				break;
			}
		}

		log->name = name;
		log->path = dir;

		if ((log->function = image_ptr(entry->function)) == NULL)
			return -3;

		if ((log->exp = image_ptr(entry->exp)) == NULL)
			return -4;

		log->line = entry->line;
	}

	return 0;
}

uint32_t ulrevbits(uint32_t v)
{
	// swap odd and even bits
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	// swap consecutive pairs
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	// swap nibbles ... 
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	// swap bytes
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	// swap 2-byte long pairs
	v = ( v >> 16             ) | ( v               << 16);

	return v;
}

int ultobin_msb_first(char * s, uint32_t val)
{
	int i;
	int n = 0;

	for (i = 31; i >=0; --i) {
		if (val & (1 << 31))
			break;
		val <<= 1;
	}

	for (n = 0; i >= 0; --i, ++n) {
		s[n] = (val & (1 << 31)) ? '1' : '0';
		val <<= 1;
	}

	s[n] = '\0';

	return n;
}

int ultobin_lsb_first(char * s, uint32_t val, int width)
{
	int i;

	for (i = 0; (i < 32) && val; ++i) {
		s[i] = (val & 1) ? '1' : '0';
		val >>= 1;
	}

	for (; (i < 32) && (i < width); ++i) {
		s[i] = '0';
	}

	s[i] = '\0';

	return i;
}


void usage(char * prog)
{
	fprintf(stderr, "Usage: %s [OPTION...] [ELF APPS]\n", prog);
	fprintf(stderr, "Expand (filter out) a DCC LOG....\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -?     \tShow this help message\n");
	fprintf(stderr, "  -l FILE\tLog file\n");
	fprintf(stderr, "  -d[d]  \tDump DCC entries\n");
	fprintf(stderr, "  -m[m]  \tDump memory blocks\n");
	fprintf(stderr, "  -s[s]  \tDump symbols\n");
	fprintf(stderr, "  -v[v]  \tVerbosity level\n");
	fprintf(stderr, "  -V     \tPrint version\n");
	fprintf(stderr, "\n");
	exit(0);
}

void version(char * prog)
{
	fprintf(stderr, "dcclog %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
	fprintf(stderr, "(C)Copyright BORESTE (www.boreste.com)\n");
	exit(1);
}

void parse_err(char * prog, char * opt)
{
	fprintf(stderr, "%s: invalid option %s\n", prog, opt);
	exit(1);
}

static int stdin_pipe_proc(FILE * stream)
{
	char buf[130];

	while (fgets(buf, 128, stdin) != NULL) {
		if (fputs(buf, stream) == EOF) {
			perror("pipe");
			break;
		}
		fflush(stream);
	};

	return 0;
}

void cleanup(void)
{
//	printf("cleanup...\n");
//	fflush(stdout);
}

void sig_quit(int signo)
{
	printf("\n");
	fflush(stdout);
	cleanup();
	exit(3);
}

int load_elf(char * pathname)
{
	char * shstrtab;
	char * symstrtab = NULL;
	Elf32_Ehdr * ehdr;
	Elf32_Shdr * shdr;
	Elf32_Phdr * phdr;
	Elf32_Sym * sym = NULL;
	char * section;
	char * name;
	FILE * f;
	int i;
	int n;

	ehdr = malloc(sizeof(Elf32_Ehdr));

	/* load binary data and symbols from elf file */
	if ((f = arm_elf_open(pathname, ehdr)) == NULL) {
		return -1;
	}

	if ((phdr = arm_elf_read_program_headers(f, ehdr)) == NULL) {
		printf(" #error: %s: arm_elf_read_program_headers()\n", __func__);
		return -6;
	}

	if (verbose) {
		arm_elf_show_program_headers(ehdr, phdr);
		fflush(stdout);
	}

	for (i = 0; i < ehdr->e_phnum; i++) {
		if (phdr[i].p_offset >= 0) {
			mem[mem_count].image = arm_elf_load_program(f, ehdr, &phdr[i]);
			if (mem[mem_count].image == NULL) {
				printf(" #error: %s: arm_elf_load_program() failed!\n", 
					   __func__);
				printf("         program header(%d): "
					   "offset=%d memsz=%d vaddr=%08x\n",  
					   i, phdr[i].p_offset, phdr[i].p_memsz, phdr[i].p_vaddr);
				return -5;
			}
			mem[mem_count].size = phdr[i].p_memsz;
			mem[mem_count].addr = phdr[i].p_vaddr;
			mem_count++;
		}
	}

	if ((shdr = arm_elf_read_section_headers(f, ehdr))  == NULL) {
		printf(" #error: %s: arm_elf_read_section_headers()\n", __func__);
		return -7;
	}

//	printf(" 2. %s: image_size=%ld\n", __func__, image_size);
//	fflush(stdout);

	if ((ehdr->e_shstrndx > 0) && (ehdr->e_shstrndx < ehdr->e_shnum)) {
		/* load section headers string table */
		shstrtab = arm_elf_load_strings(f, &shdr[ehdr->e_shstrndx]);
	} else {
		printf(" #error: %s: no section headers string table\n", __func__);
		return -1;
	};
	
	if (verbose) {
		arm_elf_show_section_headers(ehdr, shdr, shstrtab);
		fflush(stdout);
	}

	for (i = 0; i < ehdr->e_shnum; i++) {
		/* look for the symbol table section */
		if (shdr[i].sh_type == SHT_SYMTAB) {
			sym = arm_elf_load_symbols(f, &shdr[i], &n);
			if ((shdr[i].sh_link > 0) && (shdr[i].sh_link < ehdr->e_shnum)) {
				/* load section headers string table */
				symstrtab = arm_elf_load_strings(f, &shdr[shdr[i].sh_link]);
				break;
			}
			printf(" #error: %s: no symbol string table\n", __func__);
			return -1;
		}
	}

	fclose(f);

	if (sym == NULL) {
		printf(" #error: %s: no symbol table\n", __func__);
		return -1;
	}

	for (i = 0; i < n; i++) {
		if ((sym[i].st_shndx == SHN_UNDEF) || 
			(sym[i].st_shndx > SHN_LORESERVE)) {
			section = "*ABS*";
		} else {
			section = &shstrtab[shdr[sym[i].st_shndx].sh_name];
		}

		if (sym[i].st_name == 0)
			continue;

		name = &symstrtab[sym[i].st_name];

		if (name[0] == '$')
			continue;

		add_sym(section, name, sym[i].st_value, sym[i].st_size); 
	}

	free(ehdr);
	free(shdr);
	free(phdr);
	free(sym);

	return n;
}

/* Should be called from the main(). This will
 initialize signals, and signal handlers. 

 Alert: The application cannot use SIGALRM as the interval timer 
 depends on it to work.
 
 */
/* global cleanup callback */
static void (* __term_handler)(void) = NULL;

#ifdef _WIN32

BOOL CtrlHandler(DWORD fdwCtrlType) 
{ 
	switch (fdwCtrlType) { 
	case CTRL_C_EVENT: // Handle the CTRL-C signal. 
	case CTRL_BREAK_EVENT: 
	case CTRL_CLOSE_EVENT: 
		if (__term_handler != NULL) {
			__term_handler();
			return FALSE; 
		} else {
			return FALSE; 
		}

	default: 
		return FALSE; 
	} 
} 

static void __termination_handler(int signum)
{
	printf("That was all, folks\n");
	fflush(stdout);
}

#else

static void __abort_handler(int signum)
{
	const char msg[] = "\n!!! ABORTED !!!\n";
	int ret = write(STDERR_FILENO, msg, strlen(msg));
	(void)ret;
	_exit(4);
}

static void __termination_handler(int signum)
{
	struct sigaction new_action;

	/* Redirect the signal handlers to the abort handler */
	new_action.sa_handler = __abort_handler;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;

	sigaction(SIGINT, &new_action, NULL);
	sigaction(SIGTERM, &new_action, NULL);
	sigaction(SIGQUIT, &new_action, NULL);

	if (__term_handler != NULL) {
		__term_handler();
	} 

	exit(3);
}

#endif

void __term_sig_handler(void (* handler)(void))
{       
#ifdef _WIN32
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)) { 
		/* Register a cleanup callback routine */
		__term_handler = handler;
	} else {
#ifdef __MINGW32__
		__term_handler = handler;
		signal(SIGINT, __termination_handler);
		signal(SIGTERM, __termination_handler);
		signal(SIGBREAK, __termination_handler);
#endif
	}
#else
	sigset_t set;
	struct sigaction new_action;

	/* Register a cleanup callback routine */
	__term_handler = handler;

	/* Disable SIGALRM signal wich is used by the interval timer. 
	   Only one thread can have this signal enabled. */
	sigemptyset(&set);
//	sigaddset(&set, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &set, NULL);

	/* Configure the common termination handlers to call
	   the cleanup routine.  */
	new_action.sa_flags = SA_NODEFER;
	new_action.sa_handler = __termination_handler;
	sigaction(SIGINT, &new_action, NULL);
	sigaction(SIGTERM, &new_action, NULL);
	sigaction(SIGQUIT, &new_action, NULL);
#endif
}

int main(int argc, char *argv[])
{
	char appfname[1024];
	char host[128];
	int port = 1001;
	char * logfname = "-";
	int showsym = 0;
	int showdcc = 0;
	int showmem = 0;
	int host_set = 0;
	int log_set = 0;
	struct dcc_lnk lnk;
	int c;
	time_t t;
	FILE * f;

	/* the prog name start just after the last lash */
	if ((prog = (char *)strrchr(argv[0], '/')) == NULL)
		prog = argv[0];
	else
		prog++;

	/* parse the command line options */
	while ((c = getopt(argc, argv, "V?dmsvh:l:")) > 0) {
		switch (c) {
			case 'V':
				version(prog);
				break;

			case '?':
				usage(prog);
				break;

			case 'd':
				showdcc++;
				break;

			case 's':
				showsym++;
				break;

			case 'm':
				showmem++;
				break;

			case 'v':
				verbose++;
				break;

			case 'h':
				strcpy(host, optarg);
				host_set = 1;
				break;

			case 'p':
				port = strtoul(optarg, NULL, 0);
				break;
				
			case 'l':
				logfname = optarg;
				log_set = 1;
				break;

			default:
				parse_err(prog, optarg);
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "%s: missing APP name.\n\n", prog);
		usage(prog);
	}
	
	printf("\n== ARM DCC log viewer %d.%d ==\n", VERSION_MAJOR, VERSION_MINOR);
	fflush(stdout);
	 
	while (optind < argc) {
		strcpy(appfname, argv[optind]);
		printf(" - elf: %s\n", appfname);
		fflush(stdout);
		/* if elf_fd is set then force loading the elf file */
		if (load_elf(appfname) < 0) {
			return 1;
		} 
		optind++;
	}

#if defined(WIN32)
#else
	signal(SIGINT, sig_quit);
	signal(SIGTERM, sig_quit);
	signal(SIGQUIT, sig_quit);
	signal(SIGABRT, sig_quit);
#endif

	if (fix_log() < 0) {
		return 3;
	}

//	print_symbols();
//	print_mem_blocks(false);
	print_assert_entries();

	cleanup();

	return 0;
}

