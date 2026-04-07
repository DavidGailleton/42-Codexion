#include "inc/codexion.h"
#include <stdlib.h>

static t_dongle *create_dongle(int id)
{
	t_dongle *dongle;

	dongle = malloc(sizeof(t_dongle));
	dongle->id = id;
	return (dongle);
}

static t_coder *create_coder(int id, t_coder *prev_coder)
{
	t_coder *coder;

	coder = malloc(sizeof(t_coder));
	if (!coder)
		return (NULL);
	coder->id = id;
	coder->dongle_r = create_dongle(id);
	if (!coder->dongle_r)
	{
		free(coder);
		return (NULL);
	}
	if (prev_coder)
	{
		coder->dongle_l = prev_coder->dongle_r;
		prev_coder->next = coder;
	}
	else
		coder->dongle_l = NULL;
	coder->pre = prev_coder;
	coder->next = NULL;
	return (coder);
}

void free_coders(t_coder *coders)
{
	t_coder *temp;

	while (coders)
	{
		temp = coders->next;
		free(coders->dongle_r);
		free(coders);
		coders = temp;
	}
}

static t_coder *create_coders(t_config *config, t_coder *first_coder)
{
	t_coder *temp;
	int      i;

	i = 0;
	temp = first_coder;
	while (++i < config->number_of_coders)
	{
		temp = create_coder(i, temp);
		if (!temp)
		{
			free_coders(first_coder);
			return (NULL);
		}
	}
	if (config->number_of_coders > 1)
	{
		temp->next = first_coder;
		first_coder->pre = temp;
		first_coder->dongle_l = temp->dongle_r;
	}
	return (first_coder);
}

t_coder *init_coders(t_config *config)
{
	t_coder *first_coder;

	if (config->number_of_coders > 0)
		first_coder = create_coder(0, NULL);
	if (!first_coder)
		return (NULL);
	return (create_coders(config, first_coder));
}
