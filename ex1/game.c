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
#include <limits.h>

/* Global definitions */
#define NUM_CHILDREN               6
#define ERROR(...)                 { fprintf(stderr, __VA_ARGS__); }
#define ERROR_EXIT(...)            { ERROR(__VA_ARGS__); exit(EXIT_FAILURE); }
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
#define IS_FINAL_STATE(node)       ((node)->r + (node)->g + (node)->y == 0)
#define VALID_OPTION(x)            options[x]

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
void    build_next_level(node_t *node);
node_t *alloc_tree_node(node_t *parent);
node_t *create_state(node_t *parent, int caseno);
void    play_game(void);
void    print_board(node_t *node);
void    print_options(node_t *node);
int     minimax(node_t *node);
int     calculate_state_cost(node_t *node);
int     read_option(void);
void    free_memory(void);
void    free_game_tree(node_t *root);

/* Global Data */
int     M, K1, K2, K3;
node_t *root;
int     options[NUM_CHILDREN],
	next_max_choice = -1;


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
		if (!IS_FINAL_STATE(node))
			build_next_level(node->children[i]);
	}
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
	{
		node->r = node->g = node->y = M;
		node->depth = 0;
		node->parent = NULL;
	}
	else
	{
		*node = *parent;
		++(node->depth);
		node->parent = parent;
	}

	return node;
}


node_t *create_state(node_t *parent, int caseno)
{
	int     can_update = 1;
	node_t *node = alloc_tree_node(parent);

	switch (caseno)
	{
		case 0:
			if (CAN_REMOVE_RED(node, 1))
				--(node->r);
			else
				can_update = 0;
			break;
		case 1:
			if (CAN_REMOVE_GRN(node, 1))
				--(node->g);
			else
				can_update = 0;
			break;
		case 2:
			if (CAN_REMOVE_YLO(node, 1))
				--(node->y);
			else
				can_update = 0;
			break;
		case 3:
			if (CAN_REMOVE_RED(node, K1))
				node->r -= K1;
			else
				can_update = 0;
			break;
		case 4:
			if (CAN_REMOVE_GRN(node, K2))
				node->g -= K2;
			else
				can_update = 0;
			break;
		case 5:
			if (CAN_REMOVE_YLO(node, K3))
				node->y -= K3;
			else
				can_update = 0;
			break;
	}

	if (!can_update)
	{
		free(node);
		return NULL;
	}

	return node;
}


void play_game(void)
{
	node_t *currnode = root;
	int i = 0;

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
			currnode = currnode->children[next_max_choice];
		}
		else
			currnode = currnode->children[read_option()];
	}
	while (!IS_FINAL_STATE(currnode));
	printf("\n################################\n");
	printf("#    Winner is %s!!!          #\n", (i == 1) ? "MAX" : "MIN");
	printf("################################\n");
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


void print_options(node_t *node)
{
	int i;

	printf("Available options:\n");
	for (i = 0; i < NUM_CHILDREN; i++)
	{
		options[i] = 0;
		switch (i)
		{
			case 0:
				if (CAN_REMOVE_RED(node, 1))
				{
					options[i] = 1;
					PRINT_MENU_OPTION(i, 1, "RED");
				}
				break;
			case 1:
				if (CAN_REMOVE_GRN(node, 1))
				{
					options[i] = 1;
					PRINT_MENU_OPTION(i, 1, "GREEN");
				}
				break;
			case 2:
				if (CAN_REMOVE_YLO(node, 1))
				{
					options[i] = 1;
					PRINT_MENU_OPTION(i, 1, "YELLOW");
				}
				break;
			case 3:
				if (CAN_REMOVE_RED(node, K1))
				{
					options[i] = 1;
					PRINT_MENU_OPTION(i, K1, "RED");
				}
				break;
			case 4:
				if (CAN_REMOVE_GRN(node, K2))
				{
					options[i] = 1;
					PRINT_MENU_OPTION(i, K2, "GREEN");
				}
				break;
			case 5:
				if (CAN_REMOVE_YLO(node, K3))
				{
					options[i] = 1;
					PRINT_MENU_OPTION(i, K3, "YELLOW");
				}
				break;
		}
	}
}


int minimax(node_t *node)
{
	int bestvalue, i, tmp_value;

	if (IS_FINAL_STATE(node))
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
	if (IS_FINAL_STATE(node))
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


int read_option(void)
{
	int choice, i;

	do
	{
		printf("Enter choice: ");
		if (scanf("%d", &choice) != 1)
		{
			ERROR("scanf: Error reading choice\n");
		        continue;
		}
		if (choice >= 0 && choice < NUM_CHILDREN && VALID_OPTION(choice))
			break;
		ERROR("Valid options: ");
		for (i = 0; i < NUM_CHILDREN; i++)
			if (options[i])
				ERROR("%d ", i);
		ERROR("\n");
	}
	while (1);
	return choice;
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

