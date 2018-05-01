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
#define RAND_ALPHANUM(i)    ((i < d/2) ? RAND_LETTER : RAND_NUMBER)
#define ERROR(...)          { fprintf(stderr, __VA_ARGS__); }
#define ERROR_EXIT(...)     { ERROR(__VA_ARGS__); exit(EXIT_FAILURE); }
#define ERROR_RETURNV(...)  { ERROR(__VA_ARGS__); return; }

typedef struct node_s node_t;
struct node_s {
	node_t *came_from;
	float   g;
	float   e;
	char    visited;
	char   *vector;
};

typedef struct set_node_s set_node_t;
struct set_node_s {
	node_t     *node;
	set_node_t *next;
	set_node_t *prev;
};

/* Function prototypes */
void     get_args(char **argv);
void     alloc_state_space(void);
void     print_state_space(void);
void     alloc_node_array(void);
void     print_node_array(void);
void     free_state_space(int nrows);
void     free_node_array(void);
void     read_state(char *sname, node_t **nptr);
int      unique_state(int sindex);
node_t  *alloc_node(char *vector);

/* Global data */
int      L, M, d, N;
char   **state_space;
node_t **nodes;
node_t  *source, *g1, *g2, *goal;


int main(int argc, char **argv)
{
	if (argc != 5)
		ERROR_EXIT("USAGE: %s L M d N\n", argv[0]);

	get_args(argv);
//	printf("L: %d M: %d d: %d N: %d\n", L, M, d, N);

	/* Initialize pseudo-random number generator */
	srand(time(NULL));

	alloc_state_space();
	print_state_space();
	alloc_node_array();
	print_node_array();

	read_state("Source", &source);
	read_state("Goal #1", &g1);
	read_state("Goal #2", &g2);

	printf("Source:  %s\nGoal #1: %s\nGoal #2: %s\n", source->vector,
		                                g1->vector, g2->vector);

//	print_node_array();

	return EXIT_SUCCESS;
}


void get_args(char **argv)
{
	L = atoi(argv[1]);
	if (L <= 0 || L >= 10)
		ERROR_EXIT("L: should be in interval [1,9]\n");

	M = atoi(argv[2]);
	if (M <= 0 || M >= 10)
		ERROR_EXIT("M: should be in interval [1,9]\n");

	d = atoi(argv[3]);
	if (d < 2 || d % 2 != 0)
		ERROR_EXIT("d: should be in {x: EVEN(x) && x>=2}\n");

	N = atoi(argv[4]);
	// The value of N should be at least 3: 1 initial and 2 goal states
	if (N < 3 || N > (int) pow((double) L*M, (double) d/2))
		ERROR_EXIT("N: should be in {x: x>=3 && x<=(L*M)^(d/2)}"
			        " = [3, %d]\n",
			        (int) pow((double) L*M, (double) d/2));
}


void alloc_state_space(void)
{
	char **space;
	int i, j;

	if (state_space)
		ERROR_RETURNV("State space is already allocated\n");

	space = state_space = (char **) malloc(N * sizeof(char *));
	if (!space)
	{
		perror("malloc");
		exit(errno);
	}

	for (i = 0; i < N; i++)
	{
		space[i] = (char *) malloc((d+1) * sizeof(char));
		if (!space[i])
		{
			perror("malloc");
			free_state_space(i);
			exit(errno);
		}

		do
		{
			for (j = 0; j < d; j++)
				snprintf(space[i]+j, 2, "%c", RAND_ALPHANUM(j));
		}
		while (!unique_state(i));
	}
}


void print_state_space(void)
{
	int i, j;

	if (!state_space)
		ERROR_RETURNV("State space has not yet been allocated\n");

	for (i = 0; i < N; i++)
	{
#if 1
		printf("%-4d: ", i+1);
#endif
#if 1
		for (j = 0; j < d; j++)
			printf("%c ", state_space[i][j]);
		printf("\n");
#else
		printf("%s\n", state_space[i]);
#endif
	}
}


void alloc_node_array(void)
{
	node_t **narray = calloc(N, sizeof(node_t *));
	if (!narray)
	{
		perror("calloc");
		exit(errno);
	}
	nodes = narray;
}


void print_node_array(void)
{
	int i;

	if (!nodes)
		ERROR_RETURNV("Nodes array has not yet been allocated\n");

	for (i = 0; i < N; i++)
		printf("%d - %p\n", i+1, nodes[i]);
}


void free_state_space(int nrows)
{
	int i;
	for (i = 0; i < nrows; i++)
		free(state_space[i]);
	free(state_space);
}


void free_node_array(void)
{
	int i;
	for (i = 0; i < N; i++)
		if (nodes[i])
			free(nodes[i]);
	free(nodes);
}


int unique_state(int sindex)
{
	int i;

	if (!state_space)
		ERROR_EXIT("State space has not yet been allocated\n");

	for (i = 0; i < sindex; i++)
		if (strncmp(state_space[i], state_space[sindex], d) == 0)
			return 0;
	return 1;
}


void read_state(char *sname, node_t **nptr)
{
	int     x = -1, alloc = 1;
	node_t *tmp_node;

	if (!state_space)
		ERROR_RETURNV("State space has not yet been allocated!\n");

	do
	{
		do
		{
			printf("Enter state-index of %s [1-%d]: ", sname, N);
			if (scanf("%d", &x) != 1)
				ERROR("scanf: Error reading\n");
			if (x >= 1 && x <= N)
				break;
			ERROR("%d: Index out of bounds\n", x);
		}
		while (1);
	
		if (alloc)
		{
			tmp_node = alloc_node(state_space[x-1]);
			alloc = 0;
		}
		else
			tmp_node->vector = state_space[x-1];

		if (nodes[x-1])
		{
			ERROR("State #%d is already used as %s state!\n",
				x, (nodes[x-1] == source) ? "source" : "goal");
			ERROR("Please try again...\n");
		}
		else
			break;
	}
	while (1);

	*nptr = nodes[x-1] = tmp_node;
}


node_t *alloc_node(char *vector)
{
	node_t *new_node;
	new_node = malloc(sizeof(node_t));
	if (!new_node)
	{
		perror("malloc");
		exit(errno);
	}
	new_node->vector = vector;
	new_node->visited = 0;
	new_node->g = new_node->e = -1;
	return new_node;
}


