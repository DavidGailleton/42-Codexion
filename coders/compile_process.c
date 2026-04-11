#include "codexion.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

static void compiling(t_coder *coder, t_config *config)
{
	printf("%lld %d is compiling\n", get_process_time(config), coder->id);
	usleep(config->time_to_compile * 1000);
}

static void refactoring(t_coder *coder, t_config *config)
{
	printf("%lld %d is refactoring\n", get_process_time(config), coder->id);
	usleep(config->time_to_refactor * 1000);
}

static void debugging(t_coder *coder, t_config *config)
{
	printf("%lld %d is debugging\n", get_process_time(config), coder->id);
	usleep(config->time_to_debug * 1000);
}

int compile_process(t_config *config, t_coder *coder)
{
	gettimeofday(&coder->last_compile, NULL);
	compiling(coder, config);
	debugging(coder, config);
	refactoring(coder, config);
	return (0);
}
