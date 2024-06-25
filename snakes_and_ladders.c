#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY (-1)
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60
#define NUMBER_FOR_STR_L 10
#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20
#define PARAMETERS_ERROR_MSG "USAGE: this amount of parameters is not "\
"allowed\n"
/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell
{
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladder in case
    // there is one from this square
    int snake_to;  // snake_to represents the jump of the snake in case there
    // is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;
static void print_func_cell (void *cell_ptr)
{
  Cell *new_cell = (Cell *) cell_ptr;
  if (new_cell->ladder_to != EMPTY)
  {
    printf ("[%d]-ladder to %d -> ", new_cell->number,
            new_cell->ladder_to);
  }
  else if (new_cell->snake_to != EMPTY)
  {
    printf ("[%d]-snake to %d -> ", new_cell->number,
            new_cell->snake_to);
  }
  else if (new_cell->number == BOARD_SIZE)
  {
    printf ("[%d]", new_cell->number);
  }
  else
  {
    printf ("[%d] -> ", new_cell->number);
  }
}

static int comp_func_cell (void *first_cell, void *second_cell)
{
  Cell *new_first = (Cell *) first_cell;
  Cell *new_second = (Cell *) second_cell;
  if (new_first->number > new_second->number)
  {
    return 1;
  }
  if (new_first->number < new_second->number)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

static void free_func_cell (void *cell)
{
  Cell *new_cell = (Cell *) cell;
  free (new_cell);
}
static void *copy_func_cell (void *cell_ptr)
{
  Cell *new_cell_ptr = (Cell *) cell_ptr;
  Cell *new_cell = calloc (1, sizeof (Cell));
  if (new_cell == NULL)
  {
    return NULL;
  }
  *new_cell = (Cell) {new_cell_ptr->number, new_cell_ptr->ladder_to,
                      new_cell_ptr->snake_to};
  return new_cell;
}
static bool is_last_cell (void *cell)
{
  return ((Cell *) cell)->number == BOARD_SIZE;
}

/** Error handler **/
static int handle_error (char *error_msg, MarkovChain **database)
{
  printf ("%s", error_msg);
  if (database != NULL)
  {
    free_markov_chain (database);
  }
  return EXIT_FAILURE;
}

static int create_board (Cell *cells[BOARD_SIZE])
{
  for (int i = 0; i < BOARD_SIZE; i++)
  {
    cells[i] = malloc (sizeof (Cell));
    if (cells[i] == NULL)
    {
      for (int j = 0; j < i; j++)
      {
        free (cells[j]);
      }
      handle_error (ALLOCATION_ERROR_MASSAGE, NULL);
      return EXIT_FAILURE;
    }
    *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
  }

  for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
  {
    int from = transitions[i][0];
    int to = transitions[i][1];
    if (from < to)
    {
      cells[from - 1]->ladder_to = to;
    }
    else
    {
      cells[from - 1]->snake_to = to;
    }
  }
  return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database (MarkovChain *markov_chain)
{
  Cell *cells[BOARD_SIZE];
  if (create_board (cells) == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }
  MarkovNode *from_node = NULL, *to_node = NULL;
  size_t index_to;
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    add_to_database (markov_chain, cells[i]);
  }

  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    from_node = get_node_from_database (markov_chain,
                                        cells[i])->data;

    if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
    {
      index_to = MAX(cells[i]->snake_to, cells[i]->ladder_to) - 1;
      to_node = get_node_from_database (markov_chain,
                                        cells[index_to])
          ->data;
      add_node_to_counter_list (from_node, to_node,
                                markov_chain);
    }
    else
    {
      for (int j = 1; j <= DICE_MAX; j++)
      {
        index_to = ((Cell *) (from_node->data))->number + j - 1;
        if (index_to >= BOARD_SIZE)
        {
          break;
        }
        to_node = get_node_from_database (markov_chain,
                                          cells[index_to])
            ->data;
        add_node_to_counter_list (from_node,
                                  to_node, markov_chain);
      }
    }
  }
  // free temp arr
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    free (cells[i]);
  }
  return EXIT_SUCCESS;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main (int argc, char *argv[])
{
  if (argc != 3)
  {
    printf (PARAMETERS_ERROR_MSG);
    return EXIT_FAILURE;
  }
  char *str;
  int seed = (int) strtol (argv[1], &str, NUMBER_FOR_STR_L);
  int number_of_routes = (int) strtol (argv[2], &str,
                                       NUMBER_FOR_STR_L);
  srand (seed);
  MarkovChain *markov_chain_starter =
      calloc (1, sizeof (MarkovChain));
  if (markov_chain_starter == NULL)
  {
    return handle_error (ALLOCATION_ERROR_MASSAGE, &markov_chain_starter);
  }
  *markov_chain_starter = (MarkovChain) {NULL,
                                         print_func_cell, comp_func_cell,
                                         free_func_cell, copy_func_cell,
                                         is_last_cell};
  LinkedList *the_linked_list = calloc (1, sizeof (LinkedList));
  if (the_linked_list == NULL)
  {
    return handle_error (ALLOCATION_ERROR_MASSAGE, &markov_chain_starter);
  }
  *the_linked_list = (LinkedList) {NULL, NULL, 0};
  markov_chain_starter->database = the_linked_list;
  if (fill_database (markov_chain_starter) == EXIT_FAILURE)
  {
    return handle_error (ALLOCATION_ERROR_MASSAGE, &markov_chain_starter);
  }
  for (int num = 0; num < number_of_routes; num++)
  {
    printf ("Random Walk %d: ", num + 1);
    generate_random_sequence (markov_chain_starter,
                              markov_chain_starter->database->first->data,
                              MAX_GENERATION_LENGTH);
  }
  free_markov_chain (&markov_chain_starter);
  return EXIT_SUCCESS;
}
