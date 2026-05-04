/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   burnout_checker.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgaillet <dgaillet@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 20:09:20 by dgaillet          #+#    #+#             */
/*   Updated: 2026/04/21 20:09:22 by dgaillet         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

static void	burned_out(t_config *config, t_coder *coder)
{
	set_burnout(config, 1);
	pthread_mutex_lock(&config->printf_lock);
	printf("%u %d burned out\n", get_process_time(config), coder->id);
	pthread_mutex_unlock(&config->printf_lock);
}

void	wait_start(t_config *config)
{
	while (1)
	{
		pthread_mutex_lock(&config->lock);
		if (config->start)
			break ;
		pthread_mutex_unlock(&config->lock);
		usleep(100);
	}
	pthread_mutex_unlock(&config->lock);
}

void	*burnout_checker(void *arg)
{
	t_coder		*coders;
	t_config	*config;
	int			i;

	coders = (t_coder *)arg;
	config = coders->config;
	i = 0;
	wait_start(config);
	while (i++ < config->number_of_coders)
	{
		if (remain_compile(config, coders) > 0)
			i = 0;
		if (get_remain_before_burnout(config, coders) <= 0
			&& remain_compile(config, coders) > 0)
		{
			burned_out(config, coders);
			return (NULL);
		}
		coders = coders->next;
	}
	return (arg);
}
