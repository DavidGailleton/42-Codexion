#include "codexion.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

void compiling(t_coder *coder, t_config *config)
{
	gettimeofday(&coder->last_compile, NULL);
	printf("%lld %d is compiling\n", get_process_time(config), coder->id);
	usleep(config->time_to_compile * 1000);
}

void refactoring(t_coder *coder, t_config *config)
{
	printf("%lld %d is refactoring\n", get_process_time(config), coder->id);
	usleep(config->time_to_refactor * 1000);
}

void debugging(t_coder *coder, t_config *config)
{
	printf("%lld %d is debugging\n", get_process_time(config), coder->id);
	usleep(config->time_to_debug * 1000);
}
