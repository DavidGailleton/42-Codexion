#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static int has_priority(t_coder *coder, t_coder *opponent, t_config *config)
{
	if (config->scheduler == FIFO)
		return 0;
	if (config->scheduler == EDF)
	{
		if (get_remain_before_burnout(config, coder) < get_remain_before_burnout(config, opponent))
			return (1);
		else
			return (0);
	}
}

static void request_dongle(t_coder *coder, t_dongle *dongle, t_config *config)
{
	while (1)
	{
		if (!pthread_mutex_lock(&dongle->lock))
		{
			if (dongle->requester)
			{
				if (has_priority(coder, dongle->requester, config))
					return;
				else
				{
					pthread_cond_broadcast(&dongle->cond);
					pthread_cond_wait(&dongle->cond, &dongle->lock);
				}
			}
			else
			{
				dongle->requester = coder;
				pthread_cond_wait(&dongle->cond, &dongle->lock);
			}
		}
	}
}

static void release_dongle(t_dongle *dongle)
{
	dongle->requester = NULL;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}

void *thread_work(void *arg)
{
	t_coder  *coder;
	t_config *config;

	coder = (t_coder *) arg;
	config = coder->config;
	while (remain_compile(config))
	{
		increase_compiled(config);
		request_dongle(coder, coder->dongle_r, config);
		printf("%d %d has taken a dongle", get_process_time(config), coder->id);
		if (config->number_of_coders > 1)
		{
			request_dongle(coder, coder->dongle_l, config);
			printf("%d %d has taken a dongle", get_process_time(config), coder->id);
		}
		compile_process(config, coder);
		release_dongle(coder->dongle_r);
		if (config->number_of_coders > 1)
			release_dongle(coder->dongle_l);
	}
	return (NULL);
}
