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
#include <unistd.h>

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
		if (remain_compile(config, coders) > 0)
			i = 0;
		if (get_remain_before_burnout(config, coders) <= 0
			&& remain_compile(config, coders) > 0)
		{
			set_burnout(config, 1);
			pthread_mutex_lock(&config->printf_lock);
			printf("%lld %d burned out\n", get_process_time(config),
				coders->id);
			pthread_mutex_unlock(&config->printf_lock);
			return (NULL);
		}
		coders = coders->next;
	}
	return (arg);
}
