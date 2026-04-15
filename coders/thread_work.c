#include "codexion.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static int has_priority(t_coder *coder, t_config *config, t_dongle *dongle)
{
	if (config->scheduler == FIFO)
		return (1);
	if (config->scheduler == EDF && dongle->requester != NULL)
	{
		if (get_remain_before_burnout(config, coder) <=
		    get_remain_before_burnout(config, dongle->requester))
			return (1);
		dongle->requester = coder;
	}
	if (config->scheduler == EDF && dongle->requester == NULL)
		dongle->requester = coder;
	return (0);
}

static int request_dongle(t_coder *coder, t_dongle *dongle, t_config *config)
{
	struct timespec abs_burnout_t;

	abs_burnout_t = abs_time_burnout(config, coder);
	if (!dongle)
		return (1);
	pthread_mutex_lock(&dongle->lock);
	while (!has_priority(coder, config, dongle))
	{
		pthread_cond_broadcast(&dongle->cond);
		if (pthread_cond_timedwait(&dongle->cond, &dongle->lock, &abs_burnout_t) == ETIMEDOUT)
		{
			pthread_cond_broadcast(&dongle->cond);
			pthread_mutex_unlock(&dongle->lock);
			return (0);
		}
	}
	if (one_coder_burned_out(coder, config))
	{
		pthread_cond_broadcast(&dongle->cond);
		pthread_mutex_unlock(&dongle->lock);
		return (0);
	}
	printf("%lld %d has taken a dongle\n", get_process_time(config), coder->id);
	return (1);
}

static void release_dongle(t_dongle *dongle)
{
	if (!dongle)
		return;
	dongle->requester = NULL;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}

static void *work_loop(t_coder *coder, t_config *config)
{
	while (increase_compiled_if_remain(config))
	{
		if (coder->id % 2)
		{
			if (!request_dongle(coder, coder->dongle_r, config))
				return (NULL);
			if (!request_dongle(coder, coder->dongle_l, config))
				return (release_dongle(coder->dongle_r), NULL);
		}
		if (!(coder->id % 2))
		{
			if (!request_dongle(coder, coder->dongle_l, config))
				return (NULL);
			if (!request_dongle(coder, coder->dongle_r, config))
				return (release_dongle(coder->dongle_l), NULL);
		}
		compiling(coder, config);
		release_dongle(coder->dongle_r);
		release_dongle(coder->dongle_l);
		debugging(coder, config);
		refactoring(coder, config);
	}
	return (coder);
}

void *thread_work(void *arg)
{
	t_coder  *coder = NULL;
	t_config *config = NULL;

	coder = (t_coder *) arg;
	config = coder->config;
	if (!config || !coder)
		return (NULL);
	pthread_mutex_lock(&config->lock);
	while (config->start == 0)
	{
		gettimeofday(&coder->last_compile, NULL);
		pthread_cond_wait(&config->cond, &config->lock);
	}
	pthread_mutex_unlock(&config->lock);
	fprintf(stderr, "Thread %d start\n", coder->id);
	pthread_mutex_lock(&coder->lock);
	gettimeofday(&coder->last_compile, NULL);
	pthread_mutex_unlock(&coder->lock);
	return (work_loop(coder, config));
}
