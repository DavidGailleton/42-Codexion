/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   compile_process.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgaillet <dgaillet@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 20:09:36 by dgaillet          #+#    #+#             */
/*   Updated: 2026/04/21 20:09:38 by dgaillet         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

void compiling(t_coder *coder, t_config *config)
{
	pthread_mutex_lock(&coder->lock);
	coder->last_compile = get_process_time(config);
	coder->total_compile++;
	pthread_mutex_unlock(&coder->lock);
	pthread_mutex_lock(&config->printf_lock);
	if (!get_burnout(config))
		printf("%u %d is compiling\n", get_process_time(config), coder->id);
	pthread_mutex_unlock(&config->printf_lock);
	improved_usleep(config->time_to_compile, config);
}

void refactoring(t_coder *coder, t_config *config)
{
	pthread_mutex_lock(&config->printf_lock);
	if (!get_burnout(config))
		printf("%u %d is refactoring\n", get_process_time(config), coder->id);
	pthread_mutex_unlock(&config->printf_lock);
	improved_usleep(config->time_to_refactor, config);
}

void debugging(t_coder *coder, t_config *config)
{
	pthread_mutex_lock(&config->printf_lock);
	if (!get_burnout(config))
		printf("%u %d is debugging\n", get_process_time(config), coder->id);
	pthread_mutex_unlock(&config->printf_lock);
	improved_usleep(config->time_to_debug, config);
}
