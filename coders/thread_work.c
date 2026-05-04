/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread_work.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgaillet <dgaillet@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 20:10:24 by dgaillet          #+#    #+#             */
/*   Updated: 2026/04/21 20:10:25 by dgaillet         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static int	request_dongle(t_coder *coder, t_dongle *dongle, t_config *config)
{
	struct timespec	abs_burnout_t;

	if (!dongle)
		return (0);
	abs_burnout_t = abs_time_burnout(config, coder);
	pthread_mutex_lock(&dongle->lock);
	while (!has_priority(coder, config, dongle))
	{
		if (pthread_cond_timedwait(&dongle->cond, &dongle->lock,
				&abs_burnout_t) == ETIMEDOUT)
		{
			pthread_mutex_unlock(&dongle->lock);
			return (0);
		}
	}
	dongle->owner = coder;
	pthread_mutex_unlock(&dongle->lock);
	return (!get_burnout(config));
}

static void	release_dongle(t_dongle *dongle, t_config *config)
{
	if (!dongle)
		return ;
	pthread_mutex_lock(&dongle->lock);
	dongle->last_release = get_process_time(config);
	dongle->owner = NULL;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}

static int	request_dongles(t_coder *coder, t_config *config)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->id % 2)
	{
		first = coder->dongle_r;
		second = coder->dongle_l;
	}
	else
	{
		first = coder->dongle_l;
		second = coder->dongle_r;
	}
	while (!get_burnout(config))
	{
		if (!request_dongle(coder, first, config))
			return (0);
		if (request_dongle(coder, second, config))
			return (1);
		release_dongle(first, config);
		improved_usleep(1, config);
	}
	return (0);
}

static void	*work_loop(t_coder *coder, t_config *config)
{
	while (coder->total_compile < config->number_of_compiles_required
		&& !get_burnout(config))
	{
		if (!request_dongles(coder, config))
			return (NULL);
		announced_wait_dongle_take(coder->dongle_l, config, coder);
		announced_wait_dongle_take(coder->dongle_r, config, coder);
		compiling(coder, config);
		release_dongle(coder->dongle_r, config);
		release_dongle(coder->dongle_l, config);
		debugging(coder, config);
		refactoring(coder, config);
		if (get_burnout(config))
			return (NULL);
	}
	return (coder);
}

void	*thread_work(void *arg)
{
	t_coder		*coder;
	t_config	*config;

	coder = NULL;
	config = NULL;
	coder = (t_coder *)arg;
	if (!coder)
		return (NULL);
	config = coder->config;
	if (!config)
		return (NULL);
	wait_start(config);
	if (coder->id % 2 == 0)
		improved_usleep(10, config);
	return (work_loop(coder, config));
}
