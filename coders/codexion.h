#ifndef CODEXION_H
#define CODEXION_H

#include <bits/pthreadtypes.h>
#include <bits/types/struct_timeval.h>
#include <sys/time.h>
#include <sys/types.h>

#define FIFO 0
#define EDF 1

typedef struct s_config
{
	int             scheduler;
	int             number_of_coders;
	int             number_of_compiles_required;
	int             compiled;
	suseconds_t     dongle_cooldown;
	suseconds_t     time_to_burnout;
	suseconds_t     time_to_compile;
	suseconds_t     time_to_debug;
	suseconds_t     time_to_refactor;
	suseconds_t     programm_start_time;
	pthread_mutex_t lock;
} t_config;

typedef struct s_dongle
{
	int             id;
	pthread_mutex_t lock;
	void           *requester;
	pthread_cond_t  cond;
} t_dongle;

typedef struct s_coder
{
	int         id;
	suseconds_t created_at;
	int         last_compile;
	t_dongle   *dongle_r;
	t_dongle   *dongle_l;
	void       *next;
	void       *pre;
	pthread_t   thread;
	t_config   *config;
} t_coder;

t_config *parsing(int ac, char **av);
void      free_coders(t_coder *coders);
t_coder  *init_coders(t_config *config);
int       compile_process(t_config *config, t_coder *coder);
void     *thread_work(void *arg);

int get_process_time(t_config *config);
int get_remain_before_burnout(t_config *config, t_coder *coder);
int increase_compiled_if_remain(t_config *config);
int remain_compile(t_config *config);

#endif
