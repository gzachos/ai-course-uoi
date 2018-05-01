#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <math.h>

/* Global definitions */
#define RAND(max)           (rand() % (max))  /* [0,max) */
#define RAND_LETTER         ('A' + RAND(L))
#define RAND_NUMBER         ('0' + RAND(M))
#define RAND_ALPHANUM(j)    ((j < d/2) ? RAND_LETTER : RAND_NUMBER)

/* Function prototypes */
void   get_args(char **argv);
char **alloc_state_space(void);
void   print_state_space(void);
int    unique_state(int sindex);

/* Global data */
int    L, M, d, N;
char **state_space;


int main(int argc, char **argv)
{
	int i, j;

	if (argc != 5)
	{
		fprintf(stderr, "USAGE: %s L M d N\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	get_args(argv);
//	printf("L: %d M: %d d: %d N: %d\n", L, M, d, N);

	/* Initialize pseudo-random number generator */
	srand(time(NULL));

	state_space = alloc_state_space();
	print_state_space();

	return EXIT_SUCCESS;
}


void get_args(char **argv)
{
	L = atoi(argv[1]);
	if (L <= 0 || L >= 10)
	{
		fprintf(stderr, "L: should be in interval [1,9]\n");
		exit(EXIT_FAILURE);
	}

	M = atoi(argv[2]);
	if (M <= 0 || M >= 10)
	{
		fprintf(stderr, "M: should be in interval [1,9]\n");
		exit(EXIT_FAILURE);
	}

	d = atoi(argv[3]);
	if (d < 2 || d % 2 != 0)
	{
		fprintf(stderr, "d: should be in {x: EVEN(x) && x>=2}\n");
		exit(EXIT_FAILURE);
	}

	N = atoi(argv[4]);
	// The value of N should be at least 3: 1 initial and 2 goal states
	if (N < 3 || N > (int) pow((double) L*M, (double) d/2))
	{
		fprintf(stderr, "N: should be in {x: x>=3 && x<=(L*M)^(d/2)}"
			        " = [3, %d]\n",
			        (int) pow((double) L*M, (double) d/2));
		exit(EXIT_FAILURE);
	}
}


char **alloc_state_space(void)
{
	char **space;
	int i, j;

	space = state_space = (char **) malloc(N * sizeof(char *));
	if (!space)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < N; i++)
	{
		space[i] = (char *) malloc((d+1) * sizeof(char));
		if (!space[i])
		{
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		do
		{
			for (j = 0; j < d; j++)
				snprintf(space[i]+j, 2, "%c", RAND_ALPHANUM(j));
		}
		while (!unique_state(i));
	}
	return space;
}


void print_state_space(void)
{
	int i, j;
	for (i = 0; i < N; i++)
	{
//		printf("%-4d: ", i);
#if 1
		for (j = 0; j < d; j++)
			printf("%c ", state_space[i][j]);
		printf("\n");
#else
		printf("%s\n", state_space[i]);
#endif
	}
}


int unique_state(int sindex)
{
	int i;
	for (i = 0; i < sindex; i++)
		if (strncmp(state_space[i], state_space[sindex], d) == 0)
			return 0;
	return 1;
}

