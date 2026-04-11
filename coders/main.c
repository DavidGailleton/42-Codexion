#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int start_coders(t_coder *coders, t_config *config)
{
	int i;

	i = -1;
	while (++i < config->number_of_coders)
	{
		pthread_create(&coders->thread, NULL, thread_work, coders);
		coders = coders->next;
	}
	gettimeofday(&config->programm_start_time, NULL);
	config->start = 1;
	i = -1;
	while (++i < config->number_of_coders)
	{
		pthread_join(coders->thread, NULL);
		coders = coders->next;
	}
	return (1);
}

int main(int ac, char **av)
{
	t_config *config;
	t_coder  *coders;

	config = parsing(ac, av);
	if (!config)
	{
		fprintf(stderr, "%s\n", "An error occurred during parsing");
		return (1);
	}
	fprintf(stderr, "%p\n", config);
	coders = init_coders(config);
	if (!coders)
	{
		destroy(coders, config);
		fprintf(stderr, "%s\n", "An error occurred during coders initialization");
		return (1);
	}
	start_coders(coders, config);
	destroy(coders, config);
}
