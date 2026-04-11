#include "codexion.h"
#include <bits/types/struct_timeval.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>

int get_process_time(t_config *config)
{
	struct timeval time;

	gettimeofday(&time, NULL);
	return (time.tv_usec / 1000 - config->programm_start_time.tv_usec / 1000);
}

int get_remain_before_burnout(t_config *config, t_coder *coder)
{
	struct timeval time;

	gettimeofday(&time, NULL);
	return (coder->last_compile.tv_usec - config->time_to_burnout);
}

int increase_compiled_if_remain(t_config *config)
{
	while (1)
	{
		if (!pthread_mutex_lock(&config->lock))
		{
			if (!(config->compiled < config->number_of_compiles_required))
			{
				pthread_mutex_unlock(&config->lock);
				return (0);
			}
			config->compiled++;
			pthread_mutex_unlock(&config->lock);
			return (1);
		}
	}
}
