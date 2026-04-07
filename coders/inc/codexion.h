#ifndef CODEXION_H
#define CODEXION_H

typedef struct s_config
{
	int number_of_coders;
	int time_to_burnout;
	int time_to_compile;
	int time_to_debug;
	int time_to_refactor;
	int number_of_compiles_required;
	int dongle_cooldown;
	// fifo: 0; edf: 1;
	int scheduler;
} t_config;

typedef struct s_dongle
{
	int id;
	int last_compile;

} t_dongle;

typedef struct s_coder
{
	int       id;
	int       created_at;
	int       last_compile;
	t_dongle *dongle_r;
	t_dongle *dongle_l;
	void     *next;
	void     *pre;
} t_coder;

t_config *parsing(int ac, char **av);
void      free_coders(t_coder *coders);
t_coder  *init_coders(t_config *config);

#endif
