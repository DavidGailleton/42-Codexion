#include "codexion.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static int check_inputs(int ac, char **av)
{
	int i;
	int j;

	i = 0;
	while (++i < ac)
	{
		j = -1;
		if (i == ac - 1)
			if (!strcmp(av[i], "fifo") || !strcmp(av[i], "edf"))
				return (1);
		while (av[i][++j])
			if (av[i][j] < '0' || av[i][j] > '9')
				return (0);
	}
	return (0);
}

static t_config *get_config(char **av)
{
	t_config *config;

	config = malloc(sizeof(t_config));
	if (!config)
		return (NULL);
	config->number_of_coders = atoi(av[1]);
	config->time_to_burnout = atoi(av[2]);
	config->time_to_compile = atoi(av[3]);
	config->time_to_debug = atoi(av[4]);
	config->time_to_refactor = atoi(av[5]);
	config->number_of_compiles_required = atoi(av[6]);
	config->compiled = 0;
	config->dongle_cooldown = atoi(av[7]);
	config->start = 0;
	if (!strcmp(av[8], "fifo"))
		config->scheduler = FIFO;
	else if (!strcmp(av[8], "edf"))
		config->scheduler = EDF;
	else
	{
		free(config);
		return (NULL);
	}
	pthread_cond_init(&config->cond, NULL);
	pthread_mutex_init(&config->lock, NULL);
	return (config);
}

t_config *parsing(int ac, char **av)
{
	if (ac != 9 && !check_inputs(ac, av))
		return NULL;
	return (get_config(av));
}
