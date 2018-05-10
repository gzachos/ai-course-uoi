#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Global definitions */
#undef USE_GRAPHVIZ
//#define USE_GRAPHVIZ

#define NUM_CHILDREN               6
#define ERROR(...)                 { fprintf(stderr, __VA_ARGS__); }
#define ERROR_EXIT(...)            { ERROR(__VA_ARGS__); exit(EXIT_FAILURE); }
#define ERROR_RETURNV(...)         { ERROR(__VA_ARGS__); return; }
#define CAN_REMOVE_RED(node, x)    ((node)->r >= (x))
#define CAN_REMOVE_GRN(node, x)    ((node)->g >= (x))
#define CAN_REMOVE_YLO(node, x)    ((node)->y >= (x))

typedef struct node_s node_t;
struct node_s {
	int r;
	int g;
	int y;
#ifdef USE_GRAPHVIZ
	int sid;
#endif
	node_t *children[NUM_CHILDREN];
	node_t *parent;
};

/* Function Prototypes */
void    get_args(char **argv);
node_t *build_game_tree(void);
node_t *alloc_tree_node(node_t *parent);
int     is_final_state(node_t *s);
node_t *create_state(node_t *parent, int caseno);
void    build_next_level(node_t *node);
void    free_memory(void);
void    free_game_tree(node_t *root);

/* Global Data */
int     M, K1, K2, K3;
node_t *root;
#ifdef USE_GRAPHVIZ
int   sid = 0;
FILE *outfile;
#endif


int main(int argc, char **argv)
{
	if (argc != 5)
		ERROR_EXIT("USAGE: %s M K1 K2 K3\n", argv[0]);

	atexit(&free_memory);

	get_args(argv);
	root = build_game_tree();
	printf("%p\n", root);
	return EXIT_SUCCESS;
}


void get_args(char **argv)
{
	M = atoi(argv[1]);
	if (M < 3)
		ERROR_EXIT("M: should be equal to 3 or more\n");

	K1 = atoi(argv[2]);
	if (K1 <=1 || K1 >= M)
		ERROR_EXIT("K1: should be in interval [2,%d]\n", M-1);

	K2 = atoi(argv[3]);
	if (K2 <=1 || K2 >= M)
		ERROR_EXIT("K2: should be in interval [2,%d]\n", M-1);

	K3 = atoi(argv[4]);
	if (K3 <=1 || K3 >= M)
		ERROR_EXIT("K3: should be in interval [2,%d]\n", M-1);
}


node_t *build_game_tree(void)
{
#ifdef USE_GRAPHVIZ
	if (!(outfile = fopen("graph.gv", "w")))
	{
		perror("fopen");
		exit(errno);
	}
	fprintf(outfile, "strict graph {\n");
#endif
	node_t *root = alloc_tree_node(NULL);
	root->parent = NULL;
#ifdef USE_GRAPHVIZ
	root->sid = sid++;
#endif
	build_next_level(root);

#ifdef USE_GRAPHVIZ
	fprintf(outfile, "}\n");
	fclose(outfile);
	if (system("dot -Tps graph.gv -o graph.ps") == -1)
		ERROR("Error running dot using system()\n");
#endif
	return root;
}


void print_state(node_t *s)
{
	printf("%d %d %d (%p, parent: %p)\n", s->r, s->g, s->y,
			s, s->parent);
#ifdef USE_GRAPHVIZ
	if (!s->parent)
		fprintf(outfile, "\"%d: %d %d %d\"\n", s->sid, s->r, s->g, s->y);
	else
		fprintf(outfile, "\"%d: %d %d %d\" -- \"%d: %d %d %d\"\n", s->sid, s->r, s->g, s->y,
			s->parent->sid, s->parent->r, s->parent->g, s->parent->y);
#endif
}


void build_next_level(node_t *node)
{
	int i;

	if (!node)
		return;

	print_state(node);
	for (i = 0; i < NUM_CHILDREN; i++)
	{
		node->children[i] = create_state(node, i);
		if (!is_final_state(node))
			build_next_level(node->children[i]);
	}
}


node_t *create_state(node_t *parent, int caseno)
{
	int update = 1;
	node_t *node = alloc_tree_node(parent);
	switch (caseno)
	{
		case 0:
			if (CAN_REMOVE_RED(node, 1))
				--(node->r);
			else
				update = 0;
			break;
		case 1:
			if (CAN_REMOVE_GRN(node, 1))
				--(node->g);
			else
				update = 0;
			break;
		case 2:
			if (CAN_REMOVE_YLO(node, 1))
				--(node->y);
			else
				update = 0;
			break;
		case 3:
			if (CAN_REMOVE_RED(node, K1))
				node->r -= K1;
			else
				update = 0;
			break;
		case 4:
			if (CAN_REMOVE_GRN(node, K2))
				node->g -= K2;
			else
				update = 0;
			break;
		case 5:
			if (CAN_REMOVE_YLO(node, K3))
				node->y -= K3;
			else
				update = 0;
			break;
	}
#ifdef USE_GRAPHVIZ
	node->sid = sid++;
#endif
	if (!update)
	{
		free(node);
		return NULL;
	}
	node->parent = parent;
	return node;
}


node_t *alloc_tree_node(node_t *parent)
{
	node_t *node = (node_t *) malloc(sizeof(node_t));
	if (!node)
	{
		perror("malloc");
		exit(errno);
	}
	if (!parent)
		node->r = node->g = node->y = M;
	else
	{
		node->r = parent->r;
		node->g = parent->g;
		node->y = parent->y;
	}
	return node;
}


int is_final_state(node_t *s)
{
	if (s->r + s->g + s->y == 0)
		return 1;
	return 0;
}


void free_memory(void)
{
	free_game_tree(root);
}


void free_game_tree(node_t *node)
{
	int i, cnum;

	if (!node)
		return;

	for (i = cnum = 0; i < NUM_CHILDREN; i++)
		free_game_tree(node->children[i]);
	free(node);
}

