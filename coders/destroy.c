#include "codexion.h"
#include <pthread.h>
#include <stdlib.h>

static void free_dongle(t_dongle *dongle)
{
	pthread_mutex_destroy(&dongle->lock);
	pthread_cond_destroy(&dongle->cond);
	free(dongle);
}

void free_coders(t_coder *coders)
{
	t_coder *temp;

	if (coders->pre)
	{
		temp = coders->pre;
		temp->next = NULL;
	}
	while (coders)
	{
		temp = coders->next;
		if (coders->dongle_r)
			free_dongle(coders->dongle_r);
		pthread_mutex_destroy(&coders->lock);
		pthread_cond_destroy(&coders->cond);
		free(coders);
		coders = temp;
	}
}

int destroy(t_coder *coders, t_config *config)
{
	free_coders(coders);
	pthread_mutex_destroy(&config->lock);
	pthread_cond_destroy(&config->cond);
	pthread_mutex_destroy(&config->printf_lock);
	free(config);
	return (1);
}
