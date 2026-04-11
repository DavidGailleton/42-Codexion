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
		if (get_remain_before_burnout(config, coder) <= get_remain_before_burnout(config, opponent))
			return (1);
		else
			return (0);
	}
	return (0);
}

static void request_dongle(t_coder *coder, t_dongle *dongle, t_config *config)
{
	while (1)
	{
		while (pthread_mutex_lock(&dongle->lock))
		{
		}
		if (dongle->requester)
		{
			if (has_priority(coder, dongle->requester, config))
			{
				printf("%d %d has taken a dongle\n", get_process_time(config),
				       coder->id);
				return;
			}
		}
		else if (config->scheduler == FIFO)
		{
			dongle->requester = coder;
			printf("%d %d has taken a dongle\n", get_process_time(config), coder->id);
			return;
		}
		else
			dongle->requester = coder;
		pthread_mutex_unlock(&dongle->lock);
		pthread_cond_wait(&dongle->cond, &dongle->lock);
		pthread_cond_broadcast(&dongle->cond);
	}
}

static void release_dongle(t_dongle *dongle)
{
	dongle->requester = NULL;
	pthread_mutex_unlock(&dongle->lock);
	pthread_cond_broadcast(&dongle->cond);
}

void *thread_work(void *arg)
{
	t_coder  *coder;
	t_config *config;

	coder = (t_coder *) arg;
	config = coder->config;
	while (config->start == 0)
	{
	}
	while (increase_compiled_if_remain(config))
	{
		if (coder->id % 2)
		{
			request_dongle(coder, coder->dongle_r, config);
			if (config->number_of_coders > 1)
			{
				request_dongle(coder, coder->dongle_l, config);
			}
		}
		else
		{
			if (config->number_of_coders > 1)
			{
				request_dongle(coder, coder->dongle_l, config);
			}
			request_dongle(coder, coder->dongle_r, config);
		}
		compile_process(config, coder);
		release_dongle(coder->dongle_r);
		if (config->number_of_coders > 1)
			release_dongle(coder->dongle_l);
	}
	return (NULL);
}
