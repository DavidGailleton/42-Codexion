#include "codexion.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static int has_priority(t_coder *coder, t_config *config, t_dongle *dongle)
{
	long long coder_remain;
	long long req_remain;

	if (config->scheduler == FIFO)
		return (1);
	if (config->scheduler != EDF)
		return (0);
	if (dongle->requester == NULL)
	{
		dongle->requester = coder;
		return (0);
	}
	if (dongle->requester == coder)
		return (1);
	coder_remain = get_remain_before_burnout(config, coder);
	req_remain = get_remain_before_burnout(config, dongle->requester);
	if (coder_remain < req_remain)
		return (1);
	if (coder_remain == req_remain && coder->id % 2)
		return (1);
	return (0);
}

static int request_dongle(t_coder *coder, t_dongle *dongle, t_config *config)
{
	struct timespec abs_burnout_t;

	abs_burnout_t = abs_time_burnout(config, coder);
	if (!dongle)
		return (1);
	pthread_mutex_lock(&dongle->lock);
	usleep((config->dongle_cooldown * 1000LL) -
	       (get_process_time(config) -
	        (dongle->last_release.tv_sec * 1000LL + dongle->last_release.tv_usec / 1000LL)));
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
	pthread_mutex_lock(&config->printf_lock);
	if (!one_coder_burned_out(coder, config))
		printf("%lld %d has taken a dongle\n", get_process_time(config), coder->id);
	pthread_mutex_unlock(&config->printf_lock);
	return (1);
}

static void release_dongle(t_dongle *dongle, t_coder *coder)
{
	if (!dongle)
		return;
	dongle->requester = coder;
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
				return (release_dongle(coder->dongle_r, coder), NULL);
		}
		if (!(coder->id % 2))
		{
			if (!request_dongle(coder, coder->dongle_l, config))
				return (NULL);
			if (!request_dongle(coder, coder->dongle_r, config))
				return (release_dongle(coder->dongle_l, coder), NULL);
		}
		compiling(coder, config);
		release_dongle(coder->dongle_r, coder);
		release_dongle(coder->dongle_l, coder);
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
		pthread_cond_wait(&config->cond, &config->lock);
	pthread_cond_broadcast(&config->cond);
	pthread_mutex_unlock(&config->lock);
	pthread_mutex_lock(&coder->lock);
	gettimeofday(&coder->last_compile, NULL);
	pthread_mutex_unlock(&coder->lock);
	return (work_loop(coder, config));
}
