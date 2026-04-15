#include "codexion.h"
#include <errno.h>
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
	pthread_mutex_lock(&config->lock);
	if (!(config->compiled < config->number_of_compiles_required))
	{
		pthread_mutex_unlock(&config->lock);
		return (0);
	}
	config->compiled++;
	pthread_mutex_unlock(&config->lock);
	return (1);
}

int is_burnout(t_coder *coder, t_config *config)
{
	struct timespec abs_burnout_t;
	abs_burnout_t = abs_time_burnout(config, coder);

	while (pthread_mutex_lock(&coder->lock))
		if (pthread_cond_timedwait(&coder->cond, &coder->lock, &abs_burnout_t) == ETIMEDOUT)
			return (0);
	if (get_remain_before_burnout(config, coder) < 0)
	{
		pthread_mutex_unlock(&coder->lock);
		pthread_cond_broadcast(&coder->cond);
		return (1);
	}
	pthread_mutex_unlock(&coder->lock);
	pthread_cond_broadcast(&coder->cond);
	return (0);
}

struct timespec abs_time_burnout(t_config *config, t_coder *coder)
{
	struct timespec res;
	long long       usec_time;

	usec_time = coder->last_compile.tv_usec + config->time_to_burnout * 1000LL;
	res.tv_sec = coder->last_compile.tv_sec + usec_time / 1000000LL;
	res.tv_nsec = (usec_time % 1000000LL) * 1000LL;
	return res;
}
