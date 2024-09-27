#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define IMAGE_FILE "image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."
#define BOOTSIG55 0x55
#define BOOTSIGAA 0xaa
#define BOOTSIGLOCATION 510

/* Variable to store pointer to program name */
char *progname;

/* Variable to store pointer to the filename for the file being read. */
char *elfname;

/* Structure to store command line options */
static struct
{
	int vm; /*Do not use this option*/
	int extended;
} options;

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt, ...);
static void magic_signature(FILE *my_image);
static void kernel_size(FILE *my_image);
static void padding(FILE *my_image, int kernelsize);
int main(int argc, char **argv)
{
	/* Process command line options */
	progname = argv[0];
	options.vm = 0;
	options.extended = 0;
	while ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == '-'))
	{
		char *option = &argv[1][2];

		if (strcmp(option, "vm") == 0)
		{
			options.vm = 1;
		}
		else if (strcmp(option, "extended") == 0)
		{
			options.extended = 1;
		}
		else
		{
			error("%s: invalid option\nusage: %s %s\n", progname, progname, ARGS);
		}
		argc--;
		argv++;
	}
	if (options.vm == 1)
	{
		/* This option is not needed in project 1 so we doesn't bother
		 * implementing it*/
		error("%s: option --vm not implemented\n", progname);
	}
	if (argc < 3)
	{
		/* at least 3 args (createimage bootblock kernel) */
		error("usage: %s %s\n", progname, ARGS);
	}
	create_image(argc - 1, argv + 1);
	return 0;
}

static void create_image(int nfiles, char *files[])
{
	/*ELF file header and program header*/
	Elf32_Ehdr hdr;
	Elf32_Phdr phdr;

	/*Make image*/
	FILE *my_image = fopen(IMAGE_FILE, "wb");
	if (my_image == NULL)
	{
		printf("\nerror, faild to open IMAGE_FILE %s\n", IMAGE_FILE);
	}

	/*Parse through the files*/
	for (int i = 1; i <= nfiles; i++)
	{
		/*Open file[i] in rb mode, return a file pointer*/
		FILE *fp = fopen(files[i - 1], "rb");
		if (fp == NULL)
		{
			printf("error, faild to open file: %s\n", files[i - 1]);
		}

		/*First read, elf header start on 0 to fp*/
		int all_elf_header = fread(&hdr, sizeof(Elf32_Ehdr), 1, fp);
		if (all_elf_header != 1)
		{
			printf("error, coud not read\n");
		}

		/*for loop structure inspired from wiki.os.dev. Full source in raport source 2.*/
		/*Iterate through all program header*/
		for (int j = 0; j < hdr.e_phnum; j++)
		{
			/*Find location of program headers, fseek to next program header*/
			fseek(fp, (hdr.e_phoff + j * hdr.e_phentsize), SEEK_SET); /*e_phoff, where the proggram header start in relasion with elf header*/

			/*read in fp file*/
			int all_programs_headers = fread(&phdr, sizeof(Elf32_Phdr), 1, fp);
			if (all_programs_headers != 1)
			{
				printf("error, coud not read\n");
			}

			/*Allocate and write to buffer place*/
			if (phdr.p_type == 1)
			{
				char *buffer = (char *)malloc(phdr.p_memsz * sizeof(char *)); // char pointer * memsz.
				if (buffer == NULL)
				{
					printf("Faild to allocate memort to buffer.");
				}

				/*Seek offsett to program header*/
				fseek(fp, phdr.p_offset, SEEK_SET);

				/*Read from fp to buffer*/
				int transfer = fread(buffer, phdr.p_filesz, 1, fp);
				if (transfer != 1)
				{
					printf("error, coud not read\n");
				}

				/*write to my_image*/
				fwrite(buffer, 1, phdr.p_filesz, my_image);

				free(buffer);
			}
			/*Call on kernel_size function*/
			kernel_size(my_image);

			/*padding
			Never what to padd the first file*/
			if (i != 1)
			{
				padding(my_image, 5119); // padding up tp 1400. 1400 is the size of image after padding.
			}

			/*Call on magical signature*/
			magic_signature(my_image);
		}
	}

	fclose(my_image);
}

/*Magic signature 0xAA55*/
static void magic_signature(FILE *my_image)
{
	/*array with size 6*/
	char magical_sig[6] = {BOOTSIG55, BOOTSIGAA};

	/*seek end of bootbloc, 510.*/
	fseek(my_image, BOOTSIGLOCATION, SEEK_SET);

	/*write in magic_sig in in my_image*/
	fwrite(magical_sig, 2, 1, my_image);
}

/*Add padding*/
static void padding(FILE *my_image, int kernelsize)
{

	short int p = 0;
	/*Seek end of kernel, using kernel size*/
	fseek(my_image, kernelsize, SEEK_SET);

	/*Write in 0's to image*/
	fwrite(&p, 1, 1, my_image);
}

/*Kernel size, is 100% staric beacuse of int kernelSize.*/
static void kernel_size(FILE *my_image)
{

	/*Finde kernel size*/
	int kernelsize = (4791 / 512); // Hardcoded 4791/512 --> a little over 9.

	/*File pointer is on byte 2 in image*/
	fseek(my_image, 2, SEEK_SET);

	/*Write in kernel size in image*/
	fwrite(&kernelsize, 1, 1, my_image);
}

/* print an error message and exit */
static void error(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	if (errno != 0)
	{
		perror(NULL);
	}
	exit(EXIT_FAILURE);
}
