#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static int has_priority(t_coder *coder, t_coder *opponent, t_config *config)
{
	if (config->scheduler == FIFO)
		return 0;
	if (config->scheduler == EDF)
	{
		if (get_remain_before_burnout(config, coder) <= get_remain_before_burnout(config, opponent))
			return (1);
	}
	return (0);
}
static int request_dongle(t_coder *coder, t_dongle *dongle, t_config *config)
{
	while (1)
	{
		pthread_mutex_lock(&dongle->lock);
		if (dongle->requester)
		{
			if (has_priority(coder, dongle->requester, config))
			{
				printf("%lld %d has taken a dongle\n", get_process_time(config),
				       coder->id);
				return (1);
			}
		}
		else if (config->scheduler == FIFO)
		{
			dongle->requester = coder;
			printf("%lld %d has taken a dongle\n", get_process_time(config),
			       coder->id);
			return (1);
		}
		dongle->requester = coder;
		pthread_mutex_unlock(&dongle->lock);
	}
}

static void release_dongle(t_dongle *dongle)
{
	dongle->requester = NULL;
	pthread_mutex_unlock(&dongle->lock);
}

void *thread_work(void *arg)
{
	t_coder  *coder = NULL;
	t_config *config = NULL;

	coder = (t_coder *) arg;
	config = coder->config;
	if (!config || !coder)
		return (NULL);
	while (config->start == 0)
	{
	}
	gettimeofday(&coder->last_compile, NULL);
	while (increase_compiled_if_remain(config))
	{
		if (coder->id % 2)
		{
			if (!request_dongle(coder, coder->dongle_r, config))
			{
				fprintf(stderr, "%lld %d burned out\n", get_process_time(config),
				        coder->id);
				return (NULL);
			}
			if (config->number_of_coders > 1)
			{
				if (!request_dongle(coder, coder->dongle_l, config))
				{
					release_dongle(coder->dongle_r);
					fprintf(stderr, "%lld %d burned out\n", get_process_time(config),
					        coder->id);
					return (NULL);
				}
			}
		}
		else
		{
			if (config->number_of_coders > 1)
			{
				if (!request_dongle(coder, coder->dongle_l, config))
				{
					fprintf(stderr, "%lld %d burned out\n", get_process_time(config),
					        coder->id);
					return (NULL);
				}
			}
			if (!request_dongle(coder, coder->dongle_r, config))
			{
				if (config->number_of_coders > 1)
					release_dongle(coder->dongle_l);
				fprintf(stderr, "%lld %d burned out\n", get_process_time(config),
				        coder->id);
				return (NULL);
			}
		}
		gettimeofday(&coder->last_compile, NULL);
		compile_process(config, coder);
		release_dongle(coder->dongle_r);
		if (config->number_of_coders > 1)
			release_dongle(coder->dongle_l);
	}
	return (arg);
}
