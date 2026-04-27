/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgaillet <dgaillet@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 20:11:07 by dgaillet          #+#    #+#             */
/*   Updated: 2026/04/21 20:11:08 by dgaillet         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

int	remain_compile(t_config *config, t_coder *coder)
{
	int	coder_compile;
	int	compiles_required;

	pthread_mutex_lock(&coder->lock);
	coder_compile = coder->total_compile;
	pthread_mutex_unlock(&coder->lock);
	pthread_mutex_lock(&config->lock);
	compiles_required = config->number_of_compiles_required;
	pthread_mutex_unlock(&config->lock);
	return (compiles_required - coder_compile);
}

int	has_priority(t_coder *coder, t_config *config, t_dongle *dongle)
{
	long long	coder_remain;
	long long	req_remain;
	t_coder		*other_coder;

	if (dongle->owner)
		return (0);
	if (config->scheduler == FIFO)
		return (1);
	if (config->scheduler != EDF)
		return (0);
	other_coder = NULL;
	if (dongle->coder_l && dongle->coder_l != coder)
		other_coder = dongle->coder_l;
	else if (dongle->coder_r && dongle->coder_r != coder)
		other_coder = dongle->coder_r;
	if (!other_coder)
		return (1);
	coder_remain = get_remain_before_burnout(config, coder);
	req_remain = get_remain_before_burnout(config, other_coder);
	if (coder_remain < req_remain || remain_compile(config, other_coder) <= 0)
		return (1);
	if (coder_remain == req_remain && remain_compile(config,
			coder) >= remain_compile(config, other_coder))
		return (1);
	return (0);
}

t_dongle	*create_dongle(int id)
{
	t_dongle	*dongle;

	dongle = malloc(sizeof(t_dongle));
	if (!dongle)
		return (NULL);
	dongle->id = id;
	dongle->last_release.tv_sec = 0;
	dongle->last_release.tv_usec = 0;
	dongle->owner = NULL;
	dongle->coder_l = NULL;
	dongle->coder_r = NULL;
	pthread_mutex_init(&dongle->lock, NULL);
	if (pthread_cond_init(&dongle->cond, NULL))
	{
		pthread_mutex_destroy(&dongle->lock);
		free(dongle);
	}
	else
		return (dongle);
	return (NULL);
}

void	improved_usleep(unsigned int time_ms, t_config *config)
{
	while (time_ms > 0)
	{
		pthread_mutex_lock(&config->lock);
		if (config->burnout)
		{
			pthread_mutex_unlock(&config->lock);
			return ;
		}
		pthread_mutex_unlock(&config->lock);
		if (time_ms >= 10)
			usleep(10000);
		else
		{
			usleep(time_ms * 1000);
			return ;
		}
		time_ms -= 10;
	}
}
