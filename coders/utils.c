#include "codexion.h"
#include <bits/types/struct_timeval.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>

long long get_process_time(t_config *config)
{
	struct timeval time;
	long long      now_ms;
	long long      start_ms;

	gettimeofday(&time, NULL);
	now_ms = (long long) time.tv_sec * 1000LL + (long long) time.tv_usec / 1000LL;
	start_ms = (long long) config->programm_start_time.tv_sec * 1000LL +
	           (long long) config->programm_start_time.tv_usec / 1000LL;
	return (now_ms - start_ms);
}

long long get_remain_before_burnout(t_config *config, t_coder *coder)
{
	struct timeval time;
	long long      now_ms;
	long long      last_compile_ms;
	long long      elapsed_ms;

	gettimeofday(&time, NULL);
	now_ms = (long long) time.tv_sec * 1000LL + (long long) time.tv_usec / 1000LL;
	last_compile_ms = (long long) coder->last_compile.tv_sec * 1000LL +
	                  (long long) coder->last_compile.tv_usec / 1000LL;
	elapsed_ms = now_ms - last_compile_ms;
	return (config->time_to_burnout - elapsed_ms);
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
