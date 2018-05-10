#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

/* Global definitions */
#define NUM_CHILDREN               6
#define ERROR(...)                 { fprintf(stderr, __VA_ARGS__); }
#define ERROR_EXIT(...)            { ERROR(__VA_ARGS__); exit(EXIT_FAILURE); }
#define ERROR_RETURNV(...)         { ERROR(__VA_ARGS__); return; }
#define CAN_REMOVE_RED(node, x)    ((node)->r >= (x))
#define CAN_REMOVE_GRN(node, x)    ((node)->g >= (x))
#define CAN_REMOVE_YLO(node, x)    ((node)->y >= (x))
#define MAXIMIZING_PLAYER(node)    ((node)->depth % 2 == 0)
#define MIN(x,y)                   (((x) < (y)) ? (x) : (y))
#define MAX(x,y)                   (((x) > (y)) ? (x) : (y))
#define MAX_WINNING_VALUE          (3*M+1)
#define MIN_WINNING_VALUE          ((-1)*MAX_WINNING_VALUE)
#define STATE_COST(node)           calculate_state_cost(node)
#define PRINT_MENU_OPTION(x,n,c)   printf("\t%d: Remove %d %s\n", x, n, c)

typedef struct node_s node_t;
struct node_s {
	int r;
	int g;
	int y;
	int depth;
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
int     calculate_state_cost(node_t *node);
void    print_options(node_t *node);
int     read_option(void);
void    print_board(node_t *node);
int     minimax(node_t *node);
void    play_game(void);
void    print_state(node_t *s);

/* Global Data */
int     M, K1, K2, K3;
node_t *root;
int     next_max_choice = -1;


int main(int argc, char **argv)
{
	if (argc != 5)
		ERROR_EXIT("USAGE: %s M K1 K2 K3\n", argv[0]);

	atexit(&free_memory);

	get_args(argv);
	root = build_game_tree();
	play_game();

	return EXIT_SUCCESS;
}


void play_game(void)
{
	node_t *currnode = root;
	char *winner;
	short int i = 0;

	do
	{
		printf("\n############ %s's turn ##############\n",
				(i++ == 0) ? "MAX" : "MIN");
		i %= 2;
		print_board(currnode);
		print_options(currnode);
		if (MAXIMIZING_PLAYER(currnode))
		{
			minimax(currnode);
			winner = "MAX";
			currnode = currnode->children[next_max_choice];
		}
		else
		{
			winner = "MIN";
			currnode = currnode->children[read_option()];
		}
	}
	while (!is_final_state(currnode));
	printf("################################\n");
	printf("#    Winner is %s!!!          #\n", winner);
	printf("################################\n");
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
	node_t *root = alloc_tree_node(NULL);
	root->parent = NULL;
	root->depth  = 0;
	build_next_level(root);

	return root;
}


void build_next_level(node_t *node)
{
	int i;

	if (!node)
		return;

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
	if (!update)
	{
		free(node);
		return NULL;
	}
	node->parent = parent;
	node->depth = parent->depth + 1;
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


int minimax(node_t *node)
{
	int bestvalue, i, tmp_value;

	if (is_final_state(node))
		return STATE_COST(node);

	if (MAXIMIZING_PLAYER(node))
	{
		bestvalue = INT_MIN;
		for (i = 0; i < NUM_CHILDREN; i++)
		{
			if (!node->children[i])
				continue;
			tmp_value = minimax(node->children[i]);
			bestvalue = MAX(bestvalue, tmp_value);
			if (bestvalue == tmp_value)
				next_max_choice = i;
		}
		return bestvalue;
	}
	else
	{
#ifdef MIN_PLAYS_OPTIMALLY
		bestvalue = INT_MAX;
#else
		bestvalue = 0;
#endif
		for (i = 0; i < NUM_CHILDREN; i++)
		{
			if (!node->children[i])
				continue;
			tmp_value = minimax(node->children[i]);
#ifdef MIN_PLAYS_OPTIMALLY
			bestvalue = MIN(bestvalue, tmp_value);
#else
			bestvalue = MAX(bestvalue, tmp_value);
#endif
		}
		return bestvalue;
	}
}


int calculate_state_cost(node_t *node)
{
	if (is_final_state(node))
	{
		if (MAXIMIZING_PLAYER(node->parent))
#if 1
			return (MAX_WINNING_VALUE - node->depth);
#else
			return (MAX_WINNING_VALUE);
#endif
		else
#if 1
			return (MIN_WINNING_VALUE + node->depth);
#else
			return (MIN_WINNING_VALUE);
#endif
	}
	else
		ERROR_EXIT("state cost values can only be assigned to terminal nodes");
}


void print_options(node_t *node)
{
	int i;

	printf("Available options:\n");
	for (i = 0; i < NUM_CHILDREN; i++)
	{
		switch (i)
		{
			case 0:
				if (CAN_REMOVE_RED(node, 1))
					PRINT_MENU_OPTION(i, 1, "RED");
				break;
			case 1:
				if (CAN_REMOVE_GRN(node, 1))
					PRINT_MENU_OPTION(i, 1, "GREEN");
				break;
			case 2:
				if (CAN_REMOVE_YLO(node, 1))
					PRINT_MENU_OPTION(i, 1, "YELLOW");
				break;
			case 3:
				if (CAN_REMOVE_RED(node, K1))
					PRINT_MENU_OPTION(i, K1, "RED");
				break;
			case 4:
				if (CAN_REMOVE_GRN(node, K2))
					PRINT_MENU_OPTION(i, K2, "GREEN");
				break;
			case 5:
				if (CAN_REMOVE_YLO(node, K3))
					PRINT_MENU_OPTION(i, K3, "YELLOW");
				break;
		}
	}	
}


int read_option(void)
{
	int choice;
	do
	{
		printf("Enter choice: ");
		if (scanf("%d", &choice) != 1)
		{
			ERROR("scanf: Error reading choice\n");
		        continue;
		}
		if (choice >= 0 && choice < NUM_CHILDREN)
			break;
		ERROR("Valid options: [%d, %d]", 0, NUM_CHILDREN-1);
	}
	while (1);
	return choice;
}


void print_board(node_t *node)
{
	int i;
	printf("Current board state:\n\t");
	for (i = 0; i < node->r; i++)
		printf("\x1B[31m[] ");
	printf("\n\t");
	for (i = 0; i < node->g; i++)
		printf("\x1B[32m[] ");
	printf("\n\t");
	for (i = 0; i < node->y; i++)
		printf("\x1B[33m[] ");
	printf("\n");
	printf("\x1B[0m");
}

