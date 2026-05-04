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

static int	coder_started(t_coder *coder)
{
	suseconds_t	temp;

	pthread_mutex_lock(&coder->lock);
	temp = coder->last_compile;
	pthread_mutex_unlock(&coder->lock);
	if (temp < 0)
		return (0);
	return (1);
}

static void	burned_out(t_config *config, t_coder *coder)
{
	set_burnout(config, 1);
	pthread_mutex_lock(&config->printf_lock);
	printf("%u %d burned out\n", get_process_time(config), coder->id);
	pthread_mutex_unlock(&config->printf_lock);
}

void	*burnout_checker(void *arg)
{
	t_coder		*coders;
	t_config	*config;
	int			i;

	coders = (t_coder *)arg;
	config = coders->config;
	i = 0;
	while (i++ < config->number_of_coders)
	{
		if (!coder_started(coders))
		{
			coders = coders->next;
			continue ;
		}
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
