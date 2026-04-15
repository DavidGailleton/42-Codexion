#include "codexion.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static int one_coder_burned_out(t_coder *coders)
{
	t_coder *first;

	first = coders;
	pthread_mutex_lock(&coders->lock);
	if (coders->burned_out)
	{
		pthread_mutex_unlock(&coders->lock);
		return (1);
	}
	pthread_mutex_unlock(&coders->lock);
	coders = coders->next;
	while (coders && coders != first)
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

static int has_priority(t_coder *coder, t_coder *opponent, t_config *config, t_dongle *dongle)
{
	if (opponent == NULL && config->scheduler == FIFO)
	{
		dongle->requester = coder;
		return (1);
	}
	if (opponent != NULL && config->scheduler == EDF)
	{
		if (get_remain_before_burnout(config, coder) <= get_remain_before_burnout(config, opponent))
			return (1);
		dongle->requester = coder;
	}
	if (opponent == NULL && config->scheduler == EDF)
		dongle->requester = coder;
	return (0);
}

static int request_dongle(t_coder *coder, t_dongle *dongle, t_config *config)
{
	struct timespec abs_burnout_t;
	abs_burnout_t = abs_time_burnout(config, coder);

	pthread_mutex_lock(&dongle->lock);
	while (!has_priority(coder, dongle->requester, config, dongle))
	{
		pthread_cond_broadcast(&dongle->cond);
		if (pthread_cond_timedwait(&dongle->cond, &dongle->lock, &abs_burnout_t) == ETIMEDOUT || one_coder_burned_out(coder))
		{
			pthread_mutex_unlock(&dongle->lock);
			return (0);
		}
	}
	if (one_coder_burned_out(coder))
	{
		pthread_mutex_unlock(&dongle->lock);
		return (0);
	}
	printf("%lld %d has taken a dongle\n", get_process_time(config), coder->id);
	return (1);
}

static void release_dongle(t_dongle *dongle)
{
	dongle->requester = NULL;
	pthread_cond_broadcast(&dongle->cond);
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
		gettimeofday(&coder->last_compile, NULL);
	}
	gettimeofday(&coder->last_compile, NULL);
	while (increase_compiled_if_remain(config))
	{
		if (coder->id % 2)
		{
			if (!request_dongle(coder, coder->dongle_r, config) || one_coder_burned_out(coder))
				break;
			if (config->number_of_coders > 1)
			{
				if (!request_dongle(coder, coder->dongle_l, config) || one_coder_burned_out(coder))
				{
					release_dongle(coder->dongle_r);
					break;
				}
			}
		}
		else
		{
			if (config->number_of_coders > 1)
				if (!request_dongle(coder, coder->dongle_l, config) || one_coder_burned_out(coder))
					break;
			if (!request_dongle(coder, coder->dongle_r, config) || one_coder_burned_out(coder))
			{
				if (config->number_of_coders > 1)
					release_dongle(coder->dongle_l);
				break;
			}
		}
		if (one_coder_burned_out(coder))
		{
			release_dongle(coder->dongle_r);
			if (config->number_of_coders > 1)
				release_dongle(coder->dongle_l);
			return (NULL);
		}
		compiling(coder, config);
		release_dongle(coder->dongle_r);
		if (config->number_of_coders > 1)
			release_dongle(coder->dongle_l);
		debugging(coder, config);
		refactoring(coder, config);
	}
	return (arg);
}
