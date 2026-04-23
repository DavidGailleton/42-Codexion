/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgaillet <dgaillet@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 20:10:10 by dgaillet          #+#    #+#             */
/*   Updated: 2026/04/21 20:10:11 by dgaillet         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static void	create_coders(t_coder *coders, t_config *config)
{
	int		i;
	t_coder	*first;

	first = coders;
	i = 0;
	while (i < config->number_of_coders)
	{
		pthread_create(&coders->thread, NULL, thread_work, coders);
		if (i + 2 < config->number_of_coders)
			coders = ((t_coder *)(coders->next))->next;
		i += 2;
	}
	coders = first->next;
	i = 1;
	while (i < config->number_of_coders)
	{
		pthread_create(&coders->thread, NULL, thread_work, coders);
		if (i + 2 < config->number_of_coders)
			coders = ((t_coder *)(coders->next))->next;
		i += 2;
	}
}

static int	start_coders(t_coder *coders, t_config *config)
{
	int	i;

	i = 0;
	gettimeofday(&config->programm_start_time, NULL);
	create_coders(coders, config);
	pthread_create(&config->monitor, NULL, burnout_checker, coders);
	i = 0;
	while (i++ < config->number_of_coders)
	{
		pthread_join(coders->thread, NULL);
		coders = coders->next;
	}
	pthread_join(config->monitor, NULL);
	return (1);
}

int	main(int ac, char **av)
{
	t_config	*config;
	t_coder		*coders;

	config = parsing(ac, av);
	if (!config)
		return (1);
	coders = init_coders(config);
	if (!coders)
	{
		destroy(coders, config);
		fprintf(stderr, "%s\n",
			"An error occurred during coders initialization");
		return (1);
	}
	start_coders(coders, config);
	destroy(coders, config);
}
