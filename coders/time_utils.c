/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgaillet <dgaillet@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 20:10:30 by dgaillet          #+#    #+#             */
/*   Updated: 2026/04/21 20:10:32 by dgaillet         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

unsigned int get_process_time(t_config *config)
{
	struct timeval time;
	unsigned int   now_ms;
	unsigned int   start_ms;

	gettimeofday(&time, NULL);
	now_ms = (unsigned int) time.tv_sec * 1000 + (unsigned int) time.tv_usec / 1000;
	pthread_mutex_lock(&config->lock);
	start_ms = (unsigned int) config->programm_start_time.tv_sec * 1000 +
	           (unsigned int) config->programm_start_time.tv_usec / 1000;
	pthread_mutex_unlock(&config->lock);
	return (now_ms - start_ms);
}

long get_remain_before_burnout(t_config *config, t_coder *coder)
{
	long now_ms;
	long last_compile_ms;
	long elapsed_ms;

	now_ms = get_process_time(config);
	pthread_mutex_lock(&coder->lock);
	last_compile_ms = coder->last_compile;
	pthread_mutex_unlock(&coder->lock);
	elapsed_ms = now_ms - last_compile_ms;
	return (config->time_to_burnout - elapsed_ms);
}

struct timespec abs_time_burnout(t_config *config, t_coder *coder)
{
	struct timespec res;
	long long       last_compile;
	long long       total_ms;
	long long       nsec;

	pthread_mutex_lock(&coder->lock);
	last_compile = coder->last_compile;
	pthread_mutex_unlock(&coder->lock);
	total_ms = last_compile + config->time_to_burnout;
	res.tv_sec = config->programm_start_time.tv_sec + total_ms / 1000;
	nsec = (long long) config->programm_start_time.tv_usec * 1000LL + (total_ms % 1000) * 1000000LL;
	res.tv_sec += nsec / 1000000000LL;
	res.tv_nsec = nsec % 1000000000LL;
	return (res);
}

void wait_dongle_cooldown(t_config *config, t_dongle *dongle)
{
	long now_ms;
	long last_release_ms;
	long remain_ms;

	if (!config || !dongle)
		return;
	pthread_mutex_lock(&dongle->lock);
	last_release_ms = dongle->last_release;
	pthread_mutex_unlock(&dongle->lock);
	if (last_release_ms < 0)
		return;
	now_ms = get_process_time(config);
	if (now_ms - last_release_ms >= config->dongle_cooldown)
	{
		return;
	}
	remain_ms = config->dongle_cooldown - (now_ms - last_release_ms);
	improved_usleep(remain_ms, config);
}

long get_last_compile(t_coder *coder)
{
	long last_compile;

	pthread_mutex_lock(&coder->lock);
	last_compile = (long) coder->last_compile;
	pthread_mutex_unlock(&coder->lock);
	return (last_compile);
}
