#include "codexion.h"
#include <pthread.h>
#include <stdio.h>

void *burnout_checker(void *arg)
{
	t_coder  *coders;
	t_config *config;

	coders = (t_coder *) arg;
	config = coders->config;
	while (config->number_of_compiles_required > config->compiled)
	{
		pthread_mutex_lock(&coders->lock);
		if (!coders->burned_out && get_remain_before_burnout(config, coders) < 0)
		{
			printf("%lld %d burned out\n", get_process_time(config), coders->id);
			coders->burned_out = 1;
			pthread_mutex_unlock(&coders->lock);
			return (NULL);
		}
		pthread_mutex_unlock(&coders->lock);
		coders = coders->next;
	}
	return (arg);
}
