#include "codexion.h"
#include <pthread.h>
int get_burnout(t_config *config)
{
	int res;

	pthread_mutex_lock(&config->lock);
	res = config->burnout;
	pthread_mutex_unlock(&config->lock);
	return (res);
}

void set_burnout(t_config *config, int value)
{
	pthread_mutex_lock(&config->lock);
	config->burnout = value;
	pthread_mutex_unlock(&config->lock);
}
