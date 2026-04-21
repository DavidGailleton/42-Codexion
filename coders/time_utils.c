#include "codexion.h"
#include <pthread.h>
#include <unistd.h>

long long get_process_time(t_config *config)
{
	struct timeval time;
	long long      now_ms;
	long long      start_ms;

	gettimeofday(&time, NULL);
	now_ms = (long long) time.tv_sec * 1000LL + (long long) time.tv_usec / 1000LL;
	start_ms = (long long) config->programm_start_time.tv_sec * 1000LL +
	           (long long) config->programm_start_time.tv_usec / 1000LL;
	return (now_ms - start_ms);
}

long long get_remain_before_burnout(t_config *config, t_coder *coder)
{
	struct timeval     time;
	long long          now_ms;
	unsigned long long last_compile_ms;
	long long          elapsed_ms;

	gettimeofday(&time, NULL);
	now_ms = (long long) time.tv_sec * 1000LL + (long long) time.tv_usec / 1000LL;
	pthread_mutex_lock(&coder->lock);
	if (!coder->last_compile.tv_sec)
		return (1);
	last_compile_ms = (unsigned long long) coder->last_compile.tv_sec * 1000LL +
	                  (unsigned long long) coder->last_compile.tv_usec / 1000LL;
	pthread_mutex_unlock(&coder->lock);
	elapsed_ms = now_ms - last_compile_ms;
	return (config->time_to_burnout - elapsed_ms);
}

struct timespec abs_time_burnout(t_config *config, t_coder *coder)
{
	struct timespec res;
	long long       usec_time;

	pthread_mutex_lock(&coder->lock);
	usec_time = coder->last_compile.tv_usec + config->time_to_burnout * 1000LL;
	res.tv_sec = coder->last_compile.tv_sec + usec_time / 1000000LL;
	pthread_mutex_unlock(&coder->lock);
	res.tv_nsec = (usec_time % 1000000LL) * 1000LL;
	return res;
}

void wait_dongle_cooldown(t_config *config, t_dongle *dongle)
{
	long long now_ms;
	long long last_release_ms;
	long long remain_ms;

	if (!config || !dongle || dongle->last_release.tv_sec == 0)
		return;
	now_ms = get_process_time(config);
	last_release_ms =
	    dongle->last_release.tv_sec * 1000LL + dongle->last_release.tv_usec / 1000LL;
	remain_ms = config->dongle_cooldown - (now_ms - last_release_ms);
	if (remain_ms > 0)
		usleep(remain_ms * 1000);
}
