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

static int request_dongle(t_coder *coder, t_dongle *dongle, t_config *config)
{
	struct timespec abs_burnout_t;

	abs_burnout_t = abs_time_burnout(config, coder);
	if (!dongle)
		return (0);
	pthread_mutex_lock(&dongle->lock);
	while (!has_priority(coder, config, dongle))
	{
		if (pthread_cond_timedwait(&dongle->cond, &dongle->lock, &abs_burnout_t) == ETIMEDOUT)
		{
			pthread_cond_broadcast(&dongle->cond);
			pthread_mutex_unlock(&dongle->lock);
			return (0);
		}
	}
	dongle->owner = coder;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
	wait_dongle_cooldown(config, dongle);
	pthread_mutex_lock(&config->printf_lock);
	if (!get_burnout(config))
		printf("%lld %d has taken a dongle\n", get_process_time(config), coder->id);
	pthread_mutex_unlock(&config->printf_lock);
	if (get_burnout(config))
		return (0);
	return (1);
}

static void release_dongle(t_dongle *dongle)
{
	if (!dongle)
		return;
	pthread_mutex_lock(&dongle->lock);
	gettimeofday(&dongle->last_release, NULL);
	dongle->owner = NULL;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}

static void *request_dongles(t_coder *coder, t_config *config)
{
	if (coder->id % 2)
	{
		if (!request_dongle(coder, coder->dongle_r, config))
			return (NULL);
		if (!request_dongle(coder, coder->dongle_l, config))
			return (release_dongle(coder->dongle_r), NULL);
	}
	if (!(coder->id % 2))
	{
		if (!request_dongle(coder, coder->dongle_l, config))
			return (NULL);
		if (!request_dongle(coder, coder->dongle_r, config))
			return (release_dongle(coder->dongle_l), NULL);
	}
	return (coder);
}

static void *work_loop(t_coder *coder, t_config *config)
{
	while (coder->total_compile < config->number_of_compiles_required && !get_burnout(config))
	{
		if (!request_dongles(coder, config))
			return (NULL);
		compiling(coder, config);
		release_dongle(coder->dongle_r);
		release_dongle(coder->dongle_l);
		debugging(coder, config);
		refactoring(coder, config);
		if (get_burnout(config))
			return (NULL);
	}
	return (coder);
}

void *thread_work(void *arg)
{
	t_coder       *coder;
	t_config      *config;
	struct timeval time;

	coder = NULL;
	config = NULL;
	coder = (t_coder *) arg;
	if (!coder)
		return (NULL);
	config = coder->config;
	if (!config)
		return (NULL);
	pthread_mutex_lock(&config->lock);
	time = config->programm_start_time;
	pthread_mutex_unlock(&config->lock);
	pthread_mutex_lock(&coder->lock);
	coder->last_compile = time;
	pthread_mutex_unlock(&coder->lock);
	if (!(coder->id % 2))
		usleep(config->time_to_compile * 500 + config->dongle_cooldown * 500);
	return (work_loop(coder, config));
}
