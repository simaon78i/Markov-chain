#ifdef OH_TESTS
#include "/cs/usr/oryan.hassidim/ex3b/ex3b-oryan.hassidim/oh_tests/oh_tests_helpers.h"
#endif // OH_TESTS
#include "markov_chain.h"
#include <string.h>
#include <stdlib.h>

#define WORD 101
#define MAX_TWEET_NUM 20

int get_random_number (int max_number)
{
  return rand () % max_number;
}

Node *add_to_database (MarkovChain *markov_chain, void *data_ptr)
{
  if (get_node_from_database (markov_chain, data_ptr) != NULL)
  {
    Node *node_to_return = get_node_from_database (markov_chain, data_ptr);
    return node_to_return;
  }
  MarkovNode *markov_node_cur = calloc (1, sizeof (MarkovNode));
  if (markov_node_cur == NULL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    return NULL;
  }
  void *data = markov_chain->copy_func (data_ptr);
  if (data == NULL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    return NULL;
  }
  *markov_node_cur = (MarkovNode) {data,
                                   NULL, 0};
  int val = add (markov_chain->database, markov_node_cur);
  if (val == 0)
  {
    return markov_chain->database->last;
  }
  return NULL;
}

Node *get_node_from_database (MarkovChain *markov_chain, void *data_ptr)
{
  Node *current = markov_chain->database->first;
  while (current != NULL)
  {
    if (markov_chain->comp_func (current->data->data, data_ptr) == 0)
    {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

bool add_node_to_counter_list (MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain)
{
  if (markov_chain->is_last (first_node->data) == true)
  {
    return EXIT_SUCCESS;
  }
  if (first_node->counter_lst == NULL)
  {
    NextNodeCounter *frequencies_lst = calloc (1, sizeof
        (NextNodeCounter));
    if (frequencies_lst == NULL)
    {
      printf (ALLOCATION_ERROR_MASSAGE);
      return EXIT_FAILURE;
    }
    *frequencies_lst = (NextNodeCounter) {second_node,
                                          1};
    first_node->counter_lst = frequencies_lst;
    first_node->counter_lst_size++;
    return EXIT_SUCCESS;
  }
  for (int i = 0; i < first_node->counter_lst_size; i++)
  {
    if (markov_chain->comp_func (first_node->counter_lst[i].markov_node->data,
                                 second_node->data) == 0)
    {
      first_node->counter_lst[i].frequency++;
      return EXIT_SUCCESS;
    }
  }
  first_node->counter_lst_size++;
  NextNodeCounter *new_frequencies_lst = realloc
      (first_node->counter_lst,
       first_node->counter_lst_size * sizeof (NextNodeCounter));
  if (new_frequencies_lst == NULL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    free (new_frequencies_lst);
    return EXIT_FAILURE;
  }
  first_node->counter_lst = new_frequencies_lst;
  first_node->counter_lst[first_node->counter_lst_size - 1].
      frequency = 1;
  first_node->counter_lst
  [first_node->counter_lst_size - 1].markov_node = second_node;
  return EXIT_SUCCESS;
}

void free_markov_chain (MarkovChain **ptr_chain)
{
  if (ptr_chain != NULL)
  {
    if ((*ptr_chain)->database != NULL)
    {
      if ((*ptr_chain)->database->first != NULL && (*ptr_chain)
                                                       ->database->last
                                                   != NULL)
      {
        Node *cur = (*ptr_chain)->database->first;
        for (int i = 0; i < (*ptr_chain)->database->size; i++)
        {
          free (cur->data->counter_lst);
          (*ptr_chain)->free_data (cur->data->data);
          free (cur->data);
          Node *prev = cur;
          cur = cur->next;
          free (prev);
        }
      }
      free ((*ptr_chain)->database);
      (*ptr_chain)->database = NULL;
    }
    free ((*ptr_chain));
    (*ptr_chain) = NULL;
  }
}

MarkovNode *get_first_random_node (MarkovChain *markov_chain)
{
  int const num = markov_chain->database->size;
  int i;
  Node *the_node;
  do
  {
    the_node = markov_chain->database->first;
    i = get_random_number (num);
    for (int j = 1; j <= i; j++)
    {
      the_node = the_node->next;
    }
  }
  while (markov_chain->is_last (the_node->data->data) == true);
  return the_node->data;
}

MarkovNode *get_next_random_node (MarkovNode *state_struct_ptr)
{
  int total_frequencies_num = 0;
  int word_location = 0;
  for (int i = 0; i < state_struct_ptr->counter_lst_size; i++)
  {
    total_frequencies_num += state_struct_ptr->counter_lst[i].frequency;
  }
  int arg_outcome = get_random_number ((int) total_frequencies_num);
  while (arg_outcome > 0)
  {
    if (arg_outcome - state_struct_ptr->counter_lst[word_location]
        .frequency >= 0)
    {
      arg_outcome -= state_struct_ptr->counter_lst[word_location].frequency;
      word_location++;
    }
    else
    {
      break;
    }
  }
  return state_struct_ptr->counter_lst[word_location].markov_node;
}

void generate_random_sequence (MarkovChain *markov_chain, MarkovNode *
first_node, int max_length)
{
  if (first_node == NULL)
  {
    first_node = get_first_random_node (markov_chain);
  }
  markov_chain->print_func (first_node->data);
  max_length--;
  MarkovNode *next_node = get_next_random_node (first_node);
  while (--max_length > 0 && !(markov_chain->is_last (next_node->data)))
  {
    markov_chain->print_func (next_node->data);
    next_node = get_next_random_node (next_node);
  }
  markov_chain->print_func (next_node->data);
  printf ("\n");
}

