/*
 * +-----------------------------------------------------------------------+
 * |                Copyright (C) 2018 George Z. Zachos                    |
 * +-----------------------------------------------------------------------+
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact Information:
 * Name: George Z. Zachos
 * Email: gzzachos <at> gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <math.h>

/* Global definitions */
#define USE_GRAPHVIZ
#undef  DEBUG_L0
#undef  DEBUG_L1

#define RAND(max)           (rand() % (max))  /* [0,max) */
#define RAND_LETTER         ('A' + RAND(L))
#define RAND_NUMBER         ('1' + RAND(M))
#define RAND_ALPHANUM(i)    ((i < d/2) ? RAND_LETTER : RAND_NUMBER)
#define ERROR(...)          { fprintf(stderr, __VA_ARGS__); }
#define ERROR_EXIT(...)     { ERROR(__VA_ARGS__); exit(EXIT_FAILURE); }
#define ERROR_RETURNV(...)  { ERROR(__VA_ARGS__); return; }
#define IN_CLOSEDSET(x)     ((x)->visited)
#define _H(k,l)             heuristic_cost_estimate((k)->vector,(l)->vector)
#define _G(k,l)             _H(k,l)
#define EDGE_COST(x,y)      heuristic_cost_estimate(x,y) // For Graphviz
#define A_STAR(s, g, sol)   a_star(s, g, sol, 0)
#define UCS(s, g, sol)      a_star(s, g, sol, 1)
#define EXPANSION_PERC(x,y) (((x).expansions-(y).expansions)*100/((float)(x).expansions))

typedef enum search_type_e search_type_t;
enum search_type_e {A_STAR, UCS};

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

typedef struct search_solution_s search_solution_t;
struct search_solution_s {
	node_t        *source;
	node_t        *goal;
	set_node_t    *reverse_path;
	int            expansions;
	float          total_cost;
	search_type_t  search_type;
};

/* Function prototypes */
void        get_args(char **argv);
void        alloc_state_space(void);
void        print_state_space(void);
void        alloc_node_array(void);
void        print_node_array(void);
void        free_state_space(int nrows);
void        free_node_array(void);
void        read_state(char *sname, node_t **nptr);
int         unique_state(int sindex);
node_t     *alloc_node(char *vector);
float       heuristic_cost_estimate(char *v0, char *v1);
int         is_neighbor(char *x, char *y);
set_node_t *get_neighbors(node_t *node);
void        a_star(node_t *source, node_t *goal, search_solution_t *solution,
		   int ignore_heuristic);
void        print_search_solution_info(search_solution_t *s);
void        compare_search_solutions(search_solution_t x,
		search_solution_t y);
void        set_append(set_node_t **head, int *size, node_t *new_node);
int         set_contains(set_node_t **head, node_t *node);
void        free_set(set_node_t *head);
node_t     *set_pop_min_e(set_node_t **head, int *size);
node_t     *set_delete(set_node_t **head);
set_node_t *reconstruct_path(node_t *goal);
void        print_path_reverse(set_node_t *set_head);
void        reset_state_space(void);
void        free_search_solution(search_solution_t *s);
void        free_memory(void);
#ifdef USE_GRAPHVIZ
void        produce_gv_graph(void);
#endif

/* Global data */
int      L, M, d, N;
char   **state_space;
node_t **nodes;
node_t  *source, *g1, *g2;
search_solution_t s0, s1, s2, s3;


int main(int argc, char **argv)
{
	if (argc != 5)
		ERROR_EXIT("USAGE: %s L M d N\n", argv[0]);

	atexit(&free_memory);

	get_args(argv);

	/* Initialize pseudo-random number generator */
	srand(time(NULL));

	alloc_state_space();
	print_state_space();
	alloc_node_array();

#ifdef USE_GRAPHVIZ
	produce_gv_graph();
#endif

	read_state("Source", &source);
	read_state("Goal #1", &g1);
	read_state("Goal #2", &g2);

	printf("\n");
	printf("Source:  %s\n", source->vector);
	printf("Goal #1: %s\n", g1->vector);
	printf("Goal #2: %s\n", g2->vector);

	UCS(source, g1, &s0);
	UCS(source, g2, &s1);

	A_STAR(source, g1, &s2);
	A_STAR(source, g2, &s3);

	print_search_solution_info(&s0);
	print_search_solution_info(&s1);
	print_search_solution_info(&s2);
	print_search_solution_info(&s3);

	printf("\n\n######################################################\n");
	printf(    "#    Conclusion - Solution Comparison                #\n");
	printf(    "######################################################\n");
	if (s0.total_cost == -1 && s1.total_cost == -1)
		printf("No path exists from %s to %s or to %s\n\n",
				s0.source->vector, s0.goal->vector,
				s1.goal->vector);
	else if ((s0.total_cost < s1.total_cost && s0.total_cost != -1) ||
			s1.total_cost == -1)
		compare_search_solutions(s0, s2);
	else if ((s1.total_cost < s0.total_cost && s1.total_cost != -1) ||
			s0.total_cost == -1)
		compare_search_solutions(s1, s3);
	else
	{
		compare_search_solutions(s0, s2);
		compare_search_solutions(s1, s3);
	};

	return EXIT_SUCCESS;
}


void compare_search_solutions(search_solution_t x, search_solution_t y)
{
	if (x.search_type != UCS || y.search_type != A_STAR)
		ERROR_RETURNV("compare_search_solutions(x,y): "
				"x should be a UCS solution & "
				"y should be an A* search solution\n");
	if (x.source != y.source || x.goal != y.goal)
		ERROR_RETURNV("compare_search_solutions(): Source and goal "
				"states of the two solutions should match\n");
	printf("Shortest path exists between %s and %s (Total cost: %.1f)\n",
			x.source->vector, x.goal->vector, x.total_cost);
	printf("State expansions for UCS=%d and for A*=%d (%.2f%% reduction)\n\n",
			x.expansions, y.expansions, EXPANSION_PERC(x,y));
}


void get_args(char **argv)
{
	int upper_bound;

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
	upper_bound = (int) pow((double) L*M, (double) d/2);
	// The value of N and upper_bound  should be at least 3:
	// 1 source and 2 goal states
	if (upper_bound < 3)
		ERROR_EXIT("(L*M)^(d/2) = %d: should be at least 3\n",
				upper_bound);
	if (N < 3 || N > upper_bound)
		ERROR_EXIT("N: should be in {x: x>=3 && x<=(L*M)^(d/2)}"
			        " = [3, %d]\n", upper_bound);
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
	node_t **narray = (node_t **) calloc(N, sizeof(node_t *));
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
		printf("%-4d - %p\n", i+1, nodes[i]);
}


void free_state_space(int nrows)
{
	int i;
	if (!state_space)
		return;
	for (i = 0; i <= nrows; i++)
		free(state_space[i]);
	free(state_space);
}


void free_node_array(void)
{
	int i;
	if (!nodes)
		return;
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
	new_node = (node_t *) malloc(sizeof(node_t));
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


float heuristic_cost_estimate(char *v0, char *v1)
{
	int i;
	float n = 0;

	for (i = 0; i < d; i++)
		if (v0[i] != v1[i])
			n += (i < d/2) ? 1 : 0.5;
	return n;
}


int is_neighbor(char *x, char *y)
{
	int i, n;
	for (i = n = 0; i < d; i++)
		if (x[i] != y[i])
			n++;
	return (n == 1);
}


set_node_t *get_neighbors(node_t *node)
{
	node_t *tmp_node;
	set_node_t *neighbors = NULL;
	int i, size;

	for (i = 0; i < N; i++)
	{
		if (is_neighbor(node->vector, state_space[i]))
		{
			if (!nodes[i])
				tmp_node = nodes[i] = alloc_node(state_space[i]);
			else
				tmp_node = nodes[i];
			set_append(&neighbors, &size, tmp_node);
		}
	}
	return neighbors;
}


void a_star(node_t *source, node_t *goal, search_solution_t *solution,
		int ignore_heuristic)
{
	node_t     *currnode,
	           *neighbor_node;
	set_node_t *frontier = NULL,
	           *neighbor_list = NULL;
	float       new_cost;
	int         frontier_size,
		    expansions = 0;

#ifdef DEBUG_L0
	printf("\n######################################################");
	printf("\n#    Search from %s to %s", source->vector, goal->vector);
	printf("\n######################################################\n");
#endif
	source->g = 0;
	source->e = _H(source, goal);

	set_append(&frontier, &frontier_size, source);
	source->came_from = source;

	while (frontier_size > 0)
	{
#ifdef DEBUG_L0
		printf("Visit node (fs: %d): ", frontier_size);
#endif
		currnode = set_pop_min_e(&frontier, &frontier_size);
#ifdef DEBUG_L0
		printf("%s\n", currnode->vector);
#endif
		currnode->visited = 1;
		expansions++;
		if (currnode == goal)
		{
#ifdef DEBUG_L0
			printf("Reached Goal!\n");
#endif
			solution->source     = source;
			solution->goal       = goal;
			solution->expansions = expansions;
			solution->total_cost = currnode->came_from->g +
				               _G(currnode->came_from, currnode);
			solution->reverse_path = reconstruct_path(currnode);
			solution->search_type = (ignore_heuristic) ? UCS : A_STAR;

			free_set(frontier);
			free_set(neighbor_list);
			reset_state_space();
			return;
		}

		neighbor_list = get_neighbors(currnode);

#ifdef DEBUG_L0
		printf("process neighbors start\n");
#endif
		while ((neighbor_node = set_delete(&neighbor_list)) != NULL)
		{
			if (IN_CLOSEDSET(neighbor_node))
				continue;
#ifdef DEBUG_L0
			printf("\tprocess neighbor %s\n", neighbor_node->vector);
#endif
			new_cost = currnode->g + _G(currnode, neighbor_node);
#ifdef DEBUG_L1
			printf("\tnew_cost (%f) = curr->g (%f) + g(curr-neigh) (%f)\n",
					new_cost, currnode->g, _G(currnode, neighbor_node));
#endif
			if (!set_contains(&frontier, neighbor_node))
				set_append(&frontier, &frontier_size, neighbor_node);
			else if (new_cost >= neighbor_node->g)
				continue;
			neighbor_node->came_from = currnode;
			neighbor_node->g = new_cost;
			if (ignore_heuristic)
				neighbor_node->e = neighbor_node->g;
			else
				neighbor_node->e = new_cost + _H(neighbor_node, goal);
#ifdef DEBUG_L1
			printf("\te (%f) = %f + %f\n", neighbor_node->e, neighbor_node->g,
					_H(neighbor_node, goal));
#endif
		}
#ifdef DEBUG_L0
		printf("process neighbors end\n");
#endif
	}
#ifdef DEBUG_L0
	printf("\nStates: %s and %s are NOT connected!\n",
			source->vector, goal->vector);
#endif
	solution->source       = source;
	solution->goal         = goal;
	solution->expansions   = expansions;
	solution->total_cost   = -1;
	solution->reverse_path = NULL;
	solution->search_type  = (ignore_heuristic) ? UCS : A_STAR;

	free_set(frontier);
	free_set(neighbor_list);
	reset_state_space();
}


void print_search_solution_info(search_solution_t *s)
{

	char *search_name = (s->search_type == A_STAR) ?
		                  "A-star (A*)" :
		                  "Uniform Cost";
	printf("\n######################################################");
	printf("\n#    %s Search from %s to %s", search_name, s->source->vector,
			s->goal->vector);
	printf("\n######################################################\n");

	if (!s->reverse_path)
	{
		printf("\nStates: %s and %s are NOT connected!\n",
				s->source->vector, s->goal->vector);
		return;
	}
	print_path_reverse(s->reverse_path);
	printf("\nNumber of state expansions: %d\n", s->expansions);
	printf("Total (actual) path cost:   %.1f\n", s->total_cost);
}


void set_append(set_node_t **head, int *size, node_t *node)
{
	set_node_t *new_set_node,
		   *tmp_set_node;

	if (!head)
		return;

	new_set_node = (set_node_t *) malloc(sizeof(set_node_t));
	if (!new_set_node)
	{
		perror("malloc");
		exit(errno);
	}
	new_set_node->node = node;
	new_set_node->next = NULL;

	if (!(*head))
	{
		*head = new_set_node;
		new_set_node->prev = NULL;
		*size = 1;
		return;
	}

	tmp_set_node = *head;
	while (tmp_set_node->next)
		tmp_set_node = tmp_set_node->next;
	tmp_set_node->next = new_set_node;
	new_set_node->prev = tmp_set_node;
	(*size)++;
}


int set_contains(set_node_t **head, node_t *node)
{
	set_node_t *tmp_set_node;

	if (!head)
		return 0;
	if (!(*head))
		return 0;

	tmp_set_node = *head;
	while (tmp_set_node)
	{
		if (tmp_set_node->node == node)
			return 1;
		tmp_set_node = tmp_set_node->next;
	}

	return 0;
}


node_t *set_delete(set_node_t **head)
{
	node_t     *retval;
	set_node_t *tmp_set_node;

	if (!head)
		return NULL;
	if (!(*head))
		return NULL;

	retval = (*head)->node;
	if (!((*head)->next))
	{
		free(*head);
		*head = NULL;
		return retval;
	}

	tmp_set_node = *head;
	*head = (*head)->next;
	(*head)->prev = NULL;
	free(tmp_set_node);
	return retval;
}


node_t *set_pop_min_e(set_node_t **head, int *size)
{
	node_t     *retval;
	set_node_t *min_set_node,
		   *tmp_set_node;

	if (!head)
		return NULL;
	if (!(*head))
		return NULL;

	if (!(*head)->next)
	{
		retval = (*head)->node;
		free(*head);
		*head = NULL;
		(*size)--;
		return retval;
	}

	min_set_node = *head;
	tmp_set_node = (*head)->next;

	do
	{
		if (tmp_set_node->node->e < min_set_node->node->e)
			min_set_node = tmp_set_node;
		tmp_set_node = tmp_set_node->next;
	}
	while (tmp_set_node);

	if (min_set_node->prev)
		min_set_node->prev->next = min_set_node->next;
	else
		*head = min_set_node->next;

	if (min_set_node->next)
		min_set_node->next->prev = min_set_node->prev;
	retval = min_set_node->node;
	free(min_set_node);
	(*size)--;

	return retval;
}

void free_set(set_node_t *head)
{
	set_node_t *tmp_node;

	while (head)
	{
		tmp_node = head->next;
		free(head);
		head = tmp_node;
	}
}


set_node_t *reconstruct_path(node_t *goal)
{
	node_t *tmp_node = goal;
	set_node_t *rpath = NULL;
	int path_size;

	while (tmp_node && tmp_node != tmp_node->came_from)
	{
		set_append(&rpath, &path_size, tmp_node);
		tmp_node = tmp_node->came_from;
	}
	set_append(&rpath, &path_size, tmp_node);
	return rpath;
}


void print_path_reverse(set_node_t *set_head)
{
	set_node_t *tmp_set_node = set_head;
	while (tmp_set_node->next)
		tmp_set_node = tmp_set_node->next;
	printf("Solution Path: ");
	while (tmp_set_node->prev)
	{
		printf("%s -> ", tmp_set_node->node->vector);
		tmp_set_node = tmp_set_node->prev;
	}
	printf("%s\n", tmp_set_node->node->vector);
}


void reset_state_space(void)
{
	int i;
	for (i = 0; i < N; i++)
	{
		if (!nodes[i])
			continue;
		nodes[i]->visited = 0;
		nodes[i]->g = nodes[i]->e = -1;
	}
}


void free_search_solution(search_solution_t *s)
{
	if (!s)
		return;
	free_set(s->reverse_path);
}


void free_memory(void)
{
	free_node_array();
	free_state_space(N-1);
	free_search_solution(&s0);
	free_search_solution(&s1);
	free_search_solution(&s2);
	free_search_solution(&s3);
}


#ifdef USE_GRAPHVIZ
void produce_gv_graph(void)
{
	int i, j, neighbor_count;
	FILE *outfile;

	if (!(outfile = fopen("graph.gv", "w")))
	{
		perror("fopen");
		return;
	}

	fprintf(outfile, "strict graph {\n");
	for (i = 0; i < N; i++)
	{
		neighbor_count = 0;
		for (j = 0; j < N; j++)
		{
			if (is_neighbor(state_space[i], state_space[j]))
			{
				neighbor_count++;
				if (i >= j)
					continue;
				fprintf(outfile, "  \"%d: %s\" -- \"%d: %s\" "
						"[style=bold,label=\"%.1f\"]\n",
						i+1, state_space[i], j+1, state_space[j],
						EDGE_COST(state_space[i], state_space[j]));
			}
		}
		if (neighbor_count == 0)
			fprintf(outfile, "  \"%d: %s\"\n", i+1, state_space[i]);
	}
	fprintf(outfile, "}\n");
	fclose(outfile);

	if (system("dot -Tps graph.gv -o graph.ps") == -1)
		ERROR("Error running dot using system()\n");
}
#endif

