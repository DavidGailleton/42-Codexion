#include "codexion.h"
#include <stdio.h>
#include <stdlib.h>

int main(int ac, char **av)
{
	t_config *config;
	t_coder  *coders;

	config = parsing(ac, av);
	if (!config)
	{
		fprintf(stderr, "%s\n", "An error occurred during parsing");
		return (1);
	}
	coders = init_coders(config);
	if (!coders)
	{
		free(config);
		fprintf(stderr, "%s\n", "An error occurred during coders initialization");
		return (1);
	}
	free_coders(coders);
	free(config);
}
