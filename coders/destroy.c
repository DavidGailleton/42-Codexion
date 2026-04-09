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
		pthread_mutex_destroy(&coders->dongle_r->lock);
		if (coders->dongle_r)
			free_dongle(coders->dongle_r);
		free(coders);
		coders = temp;
	}
}

int destroy(t_coder *coders, t_config *config)
{
}
