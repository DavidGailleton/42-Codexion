#include "codexion.h"
#include <pthread.h>
#include <sys/time.h>

int remain_compile(t_config *config, t_coder *coder)
{
	int coder_compile;
	pthread_mutex_lock(&coder->lock);
	coder_compile = coder->total_compile;
	pthread_mutex_unlock(&coder->lock);
	return (config->number_of_compiles_required - coder_compile);
}
