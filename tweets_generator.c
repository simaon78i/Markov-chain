#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "markov_chain.h"

#define PARAMETERS_ERROR_MSG "USAGE: this amount of parameters is not "\
"allowed\n"
#define FILE_NOT_VALID_MSG "ERROR: the file path is not valid\n"
#define NUMBER_FOR_STR_TOL 10
#define SENTENCE 1001
#define MAX_TWEET_NUM 20
#define ALLOCATION_VAL 1
#define BIG_ARGS 5
#define SMALL_ARGS 4
#define WORD 101

static bool is_last_str (void *word);
/**
 * Fill the Markov chain database by reading from the given file pointer.
 * If words_to_read is EOF, reads until the end of the file.
 * Otherwise, reads the specified number of words.
 * @param fp the file pointer to read from
 * @param words_to_read the number of words to read, or EOF to read until
 * end of file
 * @param markov_chain pointer to the Markov chain to fill
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
static int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain);
/**
 * Read from the file pointer for scenarios with a small number of
 * command-line arguments.
 * This function is used when the program is called with 4 command-
 * line arguments.
 * @param fp the file pointer to read from
 * @param markov_chain pointer to the Markov chain to fill
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
static int read_for_small_args (FILE *fp, MarkovChain **markov_chain);

/**
 * Read from the file pointer for scenarios with a large number of
 * command-line arguments.
 * This function is used when the program is called with 5
 * command-line arguments.
 * @param fp the file pointer to read from
 * @param words_to_read the number of words to read
 * @param markov_chain pointer to the Markov chain to fill
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
static int read_for_big_args (FILE *fp, int words_to_read, MarkovChain
**markov_chain);

/**
 * Helper function for the main function in scenarios with a large
 * number of command-line arguments.
 * @param the_file the file pointer to read from
 * @param num_seed the seed for random number generation
 * @param num_of_tweets the number of tweets to generate
 * @param num_of_words_to_read the number of words to read from the file
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
static int main_helper_for_big_args (FILE *the_file, int num_seed, int
num_of_tweets,
                                     int num_of_words_to_read);

/**
 * Helper function for the main function in scenarios with a small
 * number of command-line arguments.
 * @param argv the command-line arguments
 * @param the_file the file pointer to read from
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
static int main_helper_for_small_args (char *const *argv, FILE *the_file);

static void print_func_str (void *ptr)
{
  if (is_last_str (ptr) == false)
  {
    printf ("%s ", (char *) ptr);
  }
  else
  {
    printf ("%s", (char *) ptr);
  }
}

static int compare_func_str (void *first, void *second)
{
  return strcmp (first, second);
}

static void free_func_str (void *ptr)
{
  free (ptr);
}

static void *copy_func_str (void *ptr_for_copy)
{
  char *new_ptr = calloc (WORD, sizeof (char));
  if (new_ptr==NULL)
  {
    return NULL;
  }
  strcpy (new_ptr, (char *) ptr_for_copy);
  return new_ptr;
}
static bool is_last_str (void *word)
{
  char *str_word = (char *) word;
  size_t length = strlen (str_word);
  if (str_word[length - 1] == '.')
  {
    return true;
  }
  return false;
}
static int read_for_big_args (FILE *fp, int words_to_read, MarkovChain
**markov_chain)
{
  char sentence[SENTENCE];
  while (fgets (sentence, SENTENCE, fp) != NULL
         && words_to_read > 0)
  {
    char *first_word;
    first_word = strtok (sentence, " \n");
    Node *first_node = NULL;
    while (first_word != NULL && words_to_read > 0)
    {
      Node *second_node = add_to_database ((*markov_chain),
                                           first_word);
      if (second_node == NULL)
      {
        return EXIT_FAILURE;
      }
      if (first_node != NULL)
      {
        int bool_val = add_node_to_counter_list (
            first_node->data, second_node->data,
            *markov_chain);
        if (bool_val == EXIT_FAILURE)
        {
          return EXIT_FAILURE;
        }
      }
      first_node = second_node;
      first_word = strtok (NULL, " \n");
      words_to_read--;
    }
  }
  return EXIT_SUCCESS;
}

static int read_for_small_args (FILE *fp, MarkovChain **markov_chain)
{
  char sentence[SENTENCE];
  while (fgets (sentence, SENTENCE, fp) != NULL)
  {
    char *first_word;
    first_word = strtok (sentence, " \n");
    Node *first_node = NULL;
    while (first_word != NULL)
    {
      Node *second_node = add_to_database ((*markov_chain), first_word);
      if (second_node == NULL)
      {
        return EXIT_FAILURE;
      }
      if (first_node != NULL)
      {
        int bool_val = add_node_to_counter_list (
            first_node->data, second_node->data, *markov_chain);
        if (bool_val == EXIT_FAILURE)
        {
          free_markov_chain (markov_chain);
          return EXIT_FAILURE;
        }
      }
      first_node = second_node;
      first_word = strtok (NULL, " \n");
    }
  }
  return EXIT_SUCCESS;
}

static int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain)
{
  if (words_to_read == EOF)
  {
    return read_for_small_args (fp, &markov_chain);
  }
  else
  {
    return read_for_big_args (fp, words_to_read, &markov_chain);
  }
}

static int main_helper_for_big_args (FILE *the_file, int num_seed, int
num_of_tweets, int num_of_words_to_read)
{
  srand (num_seed);
  MarkovChain *markov_chain_start = calloc (ALLOCATION_VAL,
                                            sizeof (MarkovChain));
  if (markov_chain_start == NULL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    fclose (the_file);
    return EXIT_FAILURE;
  }
  *markov_chain_start = (MarkovChain) {NULL, print_func_str, compare_func_str,
                                       free_func_str, copy_func_str,
                                       is_last_str};
  LinkedList *the_linked_list = calloc (ALLOCATION_VAL,
                                        sizeof (LinkedList));
  if (the_linked_list == NULL)
  {
    free_markov_chain (&markov_chain_start);
    fclose (the_file);
    printf (ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  *the_linked_list = (LinkedList) {NULL, NULL, 0};
  markov_chain_start->database = the_linked_list;
  int bool_val = fill_database (the_file,
                                num_of_words_to_read,
                                markov_chain_start);
  if (bool_val == EXIT_FAILURE)
  {
    free_markov_chain (&markov_chain_start);
    fclose (the_file);
    return EXIT_FAILURE;
  }
  int current_tweet = 1;
  while (num_of_tweets > 0)
  {
    printf ("Tweet %d: ", current_tweet);
    generate_random_sequence (markov_chain_start,
                              NULL,
                              MAX_TWEET_NUM);
    num_of_tweets--;
    current_tweet++;
  }
  free_markov_chain (&markov_chain_start);
  fclose (the_file);
  return EXIT_SUCCESS;
}

static int main_helper_for_small_args (char *const *argv, FILE *the_file)
{
  char *str;
  long seed = strtol (argv[1], &str, NUMBER_FOR_STR_TOL);
  int num_seed = (int) seed;
  long amount_of_tweets = strtol (argv[2], &str,
                                  NUMBER_FOR_STR_TOL);
  int num_of_tweets = (int) amount_of_tweets;
  srand (num_seed);
  MarkovChain *markov_chain_start = calloc (ALLOCATION_VAL,
                                            sizeof (MarkovChain));
  if (markov_chain_start == NULL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    fclose (the_file);
    return EXIT_FAILURE;
  }
  *markov_chain_start = (MarkovChain) {NULL,
                                       print_func_str, compare_func_str,
                                       free_func_str, copy_func_str,
                                       is_last_str};
  LinkedList *the_linked_list = calloc (ALLOCATION_VAL,
                                        sizeof (LinkedList));
  if (the_linked_list == NULL)
  {
    free_markov_chain (&markov_chain_start);
    fclose (the_file);
    printf (ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  *the_linked_list = (LinkedList) {NULL, NULL, 0};
  markov_chain_start->database = the_linked_list;
  int bool_val = fill_database (the_file, EOF,
                                markov_chain_start);
  if (bool_val == EXIT_FAILURE)
  {
    free_markov_chain (&markov_chain_start);
    fclose (the_file);
    return EXIT_FAILURE;
  }
  int current_tweet = 1;
  while (num_of_tweets > 0)
  {
    printf ("Tweet %d: ", current_tweet);
    generate_random_sequence (markov_chain_start,
                              NULL, MAX_TWEET_NUM);
    num_of_tweets--;
    current_tweet++;
  }
  fclose (the_file);
  return EXIT_SUCCESS;
}

int main (int argc, char *argv[])
{
  if (argc == SMALL_ARGS)
  {
    FILE *the_file = fopen (argv[3], "r");
    if (the_file == NULL)
    {
      printf (FILE_NOT_VALID_MSG);
      return EXIT_FAILURE;
    }
    return main_helper_for_small_args (argv, the_file);
  }
  if (argc == BIG_ARGS)
  {
    FILE *the_file = fopen (argv[3], "r");
    if (the_file == NULL)
    {
      printf (FILE_NOT_VALID_MSG);
      return EXIT_FAILURE;
    }
    char *str;
    long seed = strtol (argv[1], &str, NUMBER_FOR_STR_TOL);
    int num_seed = (int) seed;
    long amount_of_tweets = strtol (argv[2], &str,
                                    NUMBER_FOR_STR_TOL);
    int num_of_tweets = (int) amount_of_tweets;
    long amount_to_read = strtol (argv[4], &str,
                                  NUMBER_FOR_STR_TOL);
    int num_of_words_to_read = (int) amount_to_read;
    return main_helper_for_big_args (the_file, num_seed,
                                     num_of_tweets, num_of_words_to_read);
  }
  else
  {
    printf (PARAMETERS_ERROR_MSG);
    return EXIT_FAILURE;
  }
}

