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
	struct timeval     time;
	long long          now_ms;
	unsigned long long last_compile_ms;
	long long          elapsed_ms;

	gettimeofday(&time, NULL);
	now_ms = (long long) time.tv_sec * 1000LL + (long long) time.tv_usec / 1000LL;
	pthread_mutex_lock(&coder->lock);
	last_compile_ms = (unsigned long long) coder->last_compile.tv_sec * 1000LL +
	                  (unsigned long long) coder->last_compile.tv_usec / 1000LL;
	pthread_mutex_unlock(&coder->lock);
	elapsed_ms = now_ms - last_compile_ms;
	return (config->time_to_burnout - elapsed_ms);
}

struct timespec abs_time_burnout(t_config *config, t_coder *coder)
{
	struct timespec res;
	long long       usec_time;

	pthread_mutex_lock(&coder->lock);
	usec_time = coder->last_compile.tv_usec + config->time_to_burnout * 1000LL;
	res.tv_sec = coder->last_compile.tv_sec + usec_time / 1000000LL;
	pthread_mutex_unlock(&coder->lock);
	res.tv_nsec = (usec_time % 1000000LL) * 1000LL;
	return res;
}

int one_coder_burned_out(t_coder *coders, t_config *config)
{
	int i;

	i = 0;
	while (++i < config->number_of_coders)
	{
		pthread_mutex_lock(&coders->lock);
		if (coders->burned_out)
		{
			pthread_mutex_unlock(&coders->lock);
			return (1);
		}
		pthread_mutex_unlock(&coders->lock);
		coders = coders->next;
	}
	return (0);
}

int remain_compile(t_config *config, t_coder *coder)
{
	int coder_compile;
	pthread_mutex_lock(&coder->lock);
	coder_compile = coder->total_compile;
	pthread_mutex_unlock(&coder->lock);
	return (config->number_of_compiles_required - coder_compile);
}
