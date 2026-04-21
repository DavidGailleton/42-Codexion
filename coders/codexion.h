#ifndef CODEXION_H
#define CODEXION_H

#include <sys/time.h>
#include <sys/types.h>

#ifdef FIFO
#undef FIFO
#endif // !FIFO
#define FIFO 0

#ifdef EDF
#undef EDF
#endif // !EDF
#define EDF 1

#ifdef INVALID_INT
#undef INVALID_INT
#endif // !INVALID_INT
#define INVALID_INT 1

#ifdef INVALID_SCHEDULER
#undef INVALID_SCHEDULER
#endif // !INVALID_SCHEDULER
#define INVALID_SCHEDULER 2

#ifdef MUTEX_CRASH
#undef MUTEX_CRASH
#endif // !MUTEX_CRASH
#define MUTEX_CRASH 3

#ifdef COND_CRASH
#undef COND_CRASH
#endif // !COND_CRASH
#define COND_CRASH 4

typedef struct s_config
{
	int             scheduler;
	int             number_of_coders;
	int             number_of_compiles_required;
	int             start;
	int             burnout;
	suseconds_t     dongle_cooldown;
	suseconds_t     time_to_burnout;
	suseconds_t     time_to_compile;
	suseconds_t     time_to_debug;
	suseconds_t     time_to_refactor;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
	struct timeval  programm_start_time;
	pthread_t       monitor;
	pthread_mutex_t printf_lock;
} t_config;

typedef struct s_coder t_coder;

typedef struct s_dongle
{
	int             id;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
	struct timeval  last_release;
	t_coder        *coder_l;
	t_coder        *coder_r;
} t_dongle;

typedef struct s_coder
{
	int             id;
	int             total_compile;
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
struct timespec abs_time_burnout(t_config *config, t_coder *coder);
int             one_coder_burned_out(t_coder *coders, t_config *config);
int             remain_compile(t_config *config, t_coder *coder);

void *burnout_checker(void *arg);

void compiling(t_coder *coder, t_config *config);
void refactoring(t_coder *coder, t_config *config);
void debugging(t_coder *coder, t_config *config);

int  get_burnout(t_config *config);
void set_burnout(t_config *config, int value);

void wait_dongle_cooldown(t_config *config, t_dongle *dongle);

#endif
