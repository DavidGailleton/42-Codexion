#include "codexion.h"
#include <pthread.h>
#include <sys/time.h>

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
