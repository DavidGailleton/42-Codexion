#include "codexion.h"
#include <pthread.h>
#include <stdio.h>

void *burnout_checker(void *arg)
{
	t_coder  *coders;
	t_config *config;

	coders = (t_coder *) arg;
	config = coders->config;
	pthread_mutex_lock(&config->lock);
	while (config->number_of_compiles_required > config->compiled)
	{
		pthread_mutex_unlock(&config->lock);
		if (get_remain_before_burnout(config, coders) < 0)
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
