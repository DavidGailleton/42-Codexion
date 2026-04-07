#include "inc/codexion.h"
#include <stdio.h>

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
		fprintf(stderr, "%s\n", "An error occurred during coders initialization");
		return (1);
	}
	free_coders(coders);
}
