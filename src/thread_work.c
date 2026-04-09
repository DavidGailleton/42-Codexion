#include "codexion.h"
#include <pthread.h>

static void lock_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->dongle_l->lock);
	pthread_mutex_lock(&coder->dongle_r->lock);
}

static void unlock_dongles(t_coder *coder, t_config *config)
{
	pthread_mutex_unlock(&coder->dongle_l->lock);
	pthread_mutex_unlock(&coder->dongle_r->lock);
}

void *thread_work(void *arg)
{
	while (1)
	{
		lock_dongles(t_coder * coder);
		unlock_dongles(t_coder * coder, t_config * config);
	}
}
