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
#include <sys/time.h>

int	remain_compile(t_config *config, t_coder *coder)
{
	int	coder_compile;

	pthread_mutex_lock(&coder->lock);
	coder_compile = coder->total_compile;
	pthread_mutex_unlock(&coder->lock);
	return (config->number_of_compiles_required - coder_compile);
}
