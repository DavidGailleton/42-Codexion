/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgaillet <dgaillet@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 20:09:59 by dgaillet          #+#    #+#             */
/*   Updated: 2026/04/21 20:10:02 by dgaillet         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdlib.h>

static int add_dongles(int id, t_coder *prev_coder, t_coder *coder)
{
	coder->dongle_r = create_dongle(id);
	if (!coder->dongle_r)
		return (0);
	if (prev_coder)
	{
		coder->dongle_l = prev_coder->dongle_r;
		prev_coder->next = coder;
		coder->pre = prev_coder;
	}
	return (1);
}

static t_coder *create_coder(int id, t_coder *prev_coder, t_config *config)
{
	t_coder *coder;

	coder = malloc(sizeof(t_coder));
	if (!coder)
		return (NULL);
	coder->id = id;
	coder->total_compile = 0;
	coder->last_compile = 0;
	if (!add_dongles(id, prev_coder, coder))
		return (free(coder), NULL);
	coder->next = NULL;
	coder->config = config;
	coder->burned_out = 0;
	pthread_mutex_init(&coder->lock, NULL);
	if (pthread_cond_init(&coder->cond, NULL))
		pthread_mutex_destroy(&coder->lock);
	else
		return (coder);
	return (free(coder), NULL);
}

static void set_coder_to_dongle(t_coder *coders, t_config *config)
{
	int i;

	i = 0;
	while (i++ < config->number_of_coders)
	{
		coders->dongle_r->coder_l = coders;
		coders->dongle_r->coder_r = coders->next;
		coders = coders->next;
	}
}

static t_coder *create_coders(t_config *config, t_coder *first_coder)
{
	t_coder *temp;
	int      i;

	i = 0;
	temp = first_coder;
	while (++i < config->number_of_coders)
	{
		temp = create_coder(i + 1, temp, config);
		if (!temp)
		{
			free_coders(first_coder);
			return (NULL);
		}
	}
	if (config->number_of_coders > 1)
	{
		temp->next = first_coder;
		first_coder->pre = temp;
		first_coder->dongle_l = temp->dongle_r;
	}
	set_coder_to_dongle(first_coder, config);
	return (first_coder);
}

t_coder *init_coders(t_config *config)
{
	t_coder *first_coder;

	first_coder = NULL;
	if (config->number_of_coders > 0)
	{
		first_coder = create_coder(1, NULL, config);
		if (!first_coder)
			return (NULL);
	}
	if (config->number_of_coders == 1)
	{
		first_coder->next = first_coder;
		first_coder->pre = first_coder;
	}
	return (create_coders(config, first_coder));
}
