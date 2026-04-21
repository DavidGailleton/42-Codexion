#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check_inputs(int ac, char **av)
{
	int i;
	int j;

	i = 0;
	while (++i < ac)
	{
		if (i == ac - 1)
		{
			if (!strcmp(av[i], "fifo") || !strcmp(av[i], "edf"))
				return (0);
			else
				return (INVALID_SCHEDULER);
		}
		j = -1;
		while (av[i][++j])
			if (av[i][j] < '0' || av[i][j] > '9')
				return (INVALID_INT);
		if (strlen(av[i]) == 10 && strcmp(av[i], "2147483647") > 0)
			return (INVALID_INT);
	}
	return (-1);
}

static void basic_config_init(char **av, t_config *config)
{
	config->number_of_coders = atoi(av[1]);
	config->time_to_burnout = atoi(av[2]);
	config->time_to_compile = atoi(av[3]);
	config->time_to_debug = atoi(av[4]);
	config->time_to_refactor = atoi(av[5]);
	config->number_of_compiles_required = atoi(av[6]);
	config->dongle_cooldown = atoi(av[7]);
	config->start = 0;
}

static t_config *get_config(char **av)
{
	t_config *config;

	config = malloc(sizeof(t_config));
	if (!config)
		return (NULL);
	basic_config_init(av, config);
	if (!strcmp(av[8], "fifo"))
		config->scheduler = FIFO;
	else if (!strcmp(av[8], "edf"))
		config->scheduler = EDF;
	else
		return (free(config), NULL);
	if (pthread_cond_init(&config->cond, NULL))
		fprintf(stderr, "%s\n", "Error during config cond init");
	else if (pthread_mutex_init(&config->lock, NULL))
	{
		pthread_cond_destroy(&config->cond);
		fprintf(stderr, "%s\n", "Error during config mutex init");
	}
	else if (pthread_mutex_init(&config->printf_lock, NULL))
	{
		pthread_cond_destroy(&config->cond);
		pthread_mutex_destroy(&config->lock);
		fprintf(stderr, "%s\n", "Error during printf mutex init");
	}
	else
		return (config);
	return (free(config), NULL);
}

t_config *parsing(int ac, char **av)
{
	int res;

	if (ac != 9)
		fprintf(stderr, "%s\n", "Invalid number of args.");
	res = check_inputs(ac, av);
	if (!res)
		return (get_config(av));
	if (res == INVALID_INT)
		fprintf(stderr, "%s\n", "Invalid int in args");
	else if (res == INVALID_SCHEDULER)
		fprintf(stderr, "%s\n", "Invalid scheduler specified");
	else
		fprintf(stderr, "%s\n", "An error occured during parsing");
	return (NULL);
}
