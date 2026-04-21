#include "codexion.h"
#include <pthread.h>
#include <stdio.h>

static int remain_compile_on_every_coders(t_config *config, t_coder *coders)
{
	int i;

	i = 0;
	while (i++ < config->number_of_coders)
	{
		if (remain_compile(config, coders) <= 0)
		{
			return (0);
		}
		coders = coders->next;
	}
	return (1);
}

void *burnout_checker(void *arg)
{
	t_coder  *coders;
	t_config *config;

	coders = (t_coder *) arg;
	config = coders->config;
	pthread_mutex_lock(&config->lock);
	while (remain_compile_on_every_coders(config, coders))
	{
		pthread_mutex_unlock(&config->lock);
		if (get_remain_before_burnout(config, coders) < 0 && remain_compile(config, coders) > 0)
		{
			pthread_mutex_lock(&config->printf_lock);
			printf("%lld %d burned out\n", get_process_time(config), coders->id);
			pthread_mutex_unlock(&config->printf_lock);
			pthread_mutex_lock(&coders->lock);
			coders->burned_out = 1;
			pthread_mutex_unlock(&coders->lock);
			return (NULL);
		}
		coders = coders->next;
		pthread_mutex_lock(&config->lock);
	}
	pthread_mutex_unlock(&config->lock);
	return (arg);
}
