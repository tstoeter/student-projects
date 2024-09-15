#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "grains.h"
#include "visual.h"

void intro()
{
	printf("cuggs :: cuda grain growth simulation\n");
	printf("based on the monte carlo potts model\n");
	printf("version 1.0 by torsten stoeter, sep 2010\n\n");
}

void usage()
{
	printf("usage: ./cuggs [options]\n");
	printf("default action without any options is to simulate\na random 2d grain distribution in visual mode\n\noptions:\n");
	printf("  -help           print this help screen\n");
	printf("  -load <file>    load <file> as initial grain distribution\n");
	printf("  -kt <t>         set simulation temperature <t> as decimal value\n");
	printf("  -term <x>       terminate simulation after <x> steps\n");
	printf("  -save <file>    save final grain distribution to <file> when terminating\n");
	printf("  -stat <x>       print statistics for grain distribution every <x> steps\n");
	printf("  -verb           print verbose data on grain distribution every <x> steps\n");
	printf("  -dump <x>       dump grain distribution to disk every <x> steps\n");
	printf("  -visual         enable visualization\n");
}

char *loadfile = NULL, *savefile = NULL;

int main(int argc, char *argv[])
{
	intro();

	if (argc == 1)
	{
		visual = 1;
	}
	else
	{
		int i = 0;

		while (++i < argc)
		{
			if (strncmp(argv[i], "-help", 80) == 0)
			{
				usage();
				exit(0);
			}
			else if (strncmp(argv[i], "-load", 80) == 0)
			{
				if (++i < argc)
					loadfile = argv[i];
			}
			else if (strncmp(argv[i], "-save", 80) == 0)
			{
				if (++i < argc)
					savefile = argv[i];
			}
			else if (strncmp(argv[i], "-kt", 80) == 0)
			{
				if (++i < argc)
					kT = atof(argv[i]);

				if (kT > 0)
				{
					printf("kT = %f\n", kT);
				}
				else
				{
					printf("bad kt: %f\n", kT);
					exit(0);
				}
			}

			else if (strncmp(argv[i], "-term", 80) == 0)
			{
				if (++i < argc)
					term = atoi(argv[i]);

				if (term < 0 && term > INT_MAX)
				{
					printf("bad term step: %d\n", term);
					exit(0);
				}
			}
			else if (strncmp(argv[i], "-stat", 80) == 0)
			{
				if (++i < argc)
					stat = atoi(argv[i]);

				if (stat < 0 && stat > INT_MAX)
				{
					printf("bad stat step: %d\n", stat);
					exit(0);
				}
			}
			else if (strncmp(argv[i], "-dump", 80) == 0)
			{
				if (++i < argc)
					dump = atoi(argv[i]);

				if (dump < 0 && dump > INT_MAX)
				{
					printf("bad dump step: %d\n", dump);
					exit(0);
				}
			}
			else if (strncmp(argv[i], "-verb", 80) == 0)
			{
				verb = 1;
			}
			else if (strncmp(argv[i], "-visual", 80) == 0)
			{
				visual = 1;
			}
			else
			{
				printf("bad option: %s\n", argv[i]);
				exit(0);
			}
		}
	}

	run_cuggs(argc, argv, loadfile, savefile);

	return 0;
}

