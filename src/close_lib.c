#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

static char file_close[] = "close.out";

typedef int (*orig_close_f_type)(int);

orig_close_f_type orig_close = NULL;

bool close_catched = false;

pthread_mutex_t close_mutex;

struct close_call {
	unsigned long int timestamp;
	int fd;
	int ret_code;
};

#define MAX_CALLS 256

static thread_local struct __close_array {
    struct close_call all_calls[MAX_CALLS];
	int curr_call;
} __close_collection;

void dump_close(FILE* file, struct close_call* call)
{
	fprintf(file, "%lu %d %d", call->timestamp, call->fd, call->ret_code);
}

int close(int fd)
{
	 __close_collection.curr_call = 0;
	 __close_collection.all_calls[__close_collection.curr_call].timestamp = (unsigned long int)time(NULL);


		if (orig_close == NULL)
	{
		pthread_mutex_lock(&close_mutex);
		if (close_catched == false)
		{
			orig_close = dlsym(RTLD_NEXT, "close");
			close_catched = true;
		}
		pthread_mutex_unlock(&close_mutex);
	}
	if(orig_close == NULL) printf("%lu\n", orig_close);
	__close_collection.all_calls[__close_collection.curr_call].fd = fd;
	int ret_code = orig_close(fd);
	__close_collection.all_calls[__close_collection.curr_call].ret_code = ret_code;
	__close_collection.curr_call++;

	if (__close_collection.curr_call == MAX_CALLS)
	{
		pthread_mutex_lock(&close_mutex);

		FILE *f = fopen(file_close, "w");
		for (int i = 0; i < MAX_CALLS; ++i)
		{
			dump_close(f, &__close_collection.all_calls[i]);
		}

		pthread_mutex_unlock(&close_mutex);
		__close_collection.curr_call = 0;
	}

	return ret_code;
}
