#ifndef CODEXION_H
#define CODEXION_H

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
	int             start;
	suseconds_t     dongle_cooldown;
	suseconds_t     time_to_burnout;
	suseconds_t     time_to_compile;
	suseconds_t     time_to_debug;
	suseconds_t     time_to_refactor;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
	struct timeval  programm_start_time;
	pthread_t       monitor;
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
	int             id;
	suseconds_t     created_at;
	struct timeval  last_compile;
	t_dongle       *dongle_r;
	t_dongle       *dongle_l;
	void           *next;
	void           *pre;
	pthread_t       thread;
	t_config       *config;
	pthread_cond_t  cond;
	pthread_mutex_t lock;
	int             burned_out;
} t_coder;

t_config *parsing(int ac, char **av);
void      free_coders(t_coder *coders);
t_coder  *init_coders(t_config *config);
void     *thread_work(void *arg);
int       destroy(t_coder *coders, t_config *config);

long long       get_process_time(t_config *config);
long long       get_remain_before_burnout(t_config *config, t_coder *coder);
int             increase_compiled_if_remain(t_config *config);
struct timespec abs_time_burnout(t_config *config, t_coder *coder);
int             one_coder_burned_out(t_coder *coders, t_config *config);

void *burnout_checker(void *arg);

void compiling(t_coder *coder, t_config *config);
void refactoring(t_coder *coder, t_config *config);
void debugging(t_coder *coder, t_config *config);

#endif
