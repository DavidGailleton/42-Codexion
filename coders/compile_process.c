#include "codexion.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

static void compiling(t_coder *coder, t_config *config)
{
	printf("%d %d is compiling\n", get_process_time(config), coder->id);
	usleep(config->time_to_compile);
}

static void refactoring(t_coder *coder, t_config *config)
{
	printf("%d %d is refactoring\n", get_process_time(config), coder->id);
	usleep(config->time_to_refactor);
}

static void debugging(t_coder *coder, t_config *config)
{
	printf("%d %d is debugging\n", get_process_time(config), coder->id);
	usleep(config->time_to_debug);
}

int compile_process(t_config *config, t_coder *coder)
{
	compiling(coder, config);
	refactoring(coder, config);
	debugging(coder, config);
	return (0);
}
