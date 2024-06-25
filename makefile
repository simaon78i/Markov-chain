CC = gcc
Flags = -Wall -Wextra -Wvla -std=c99 -Werror
tweets: markov_chain.o tweets_generator.o linked_list.o
	gcc tweets_generator.o markov_chain.o linked_list.o -o tweets_generator
snake:  markov_chain.o snakes_and_ladders.o linked_list.o
	gcc  snakes_and_ladders.o markov_chain.o linked_list.o -o snakes_and_ladders
linked_list.o: linked_list.c linked_list.h
	gcc -c $(Flages) $<
markov_chain.o: markov_chain.h markov_chain.c linked_list.h
	gcc -c $(Flages) $^
tweets_generator.o: markov_chain.h tweets_generator.c linked_list.h
	gcc -c $(Flages) $^
snakes_and_ladders.o: markov_chain.h snakes_and_ladders.c linked_list.h
	gcc -c $(Flages) $^
