#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

typedef int (*orig_open_f_type)(char*, int, ...);

orig_open_f_type orig_open = NULL;

bool open_catched = false;

pthread_mutex_t open_mutex;

struct open_call {
	unsigned long int timestamp;
	char *name;
	int flag;
	int mode;
	int fd;
};

#define MAX_CALLS 256

static thread_local struct __open_array {
	struct open_call all_calls[MAX_CALLS];
	int curr_call;
} __open_collection;


bool check_mode(int mode_) 
{
    return ( mode_ == S_IRWXU || mode_ == S_IRUSR ||
 	     mode_ == S_IWUSR || mode_ == S_IXUSR ||
	     mode_ == S_IRWXG || mode_ == S_IRGRP ||
	     mode_ == S_IWGRP || mode_ == S_IXGRP ||
	     mode_ == S_IRWXO || mode_ == S_IROTH ||
	     mode_ == S_IWOTH || mode_ == S_IXOTH ||
	     mode_ == S_ISUID || mode_ == S_ISGID ||
	     mode_ == S_ISVTX );
}

void dump_open(FILE *f, struct open_call *call)
{
	fprintf(f, "%lu %s %d %d %d", call->timestamp, call->name, call->flag, 
					call->mode, call->fd);
}

int open(char *name, int flag, ...)
{
	__open_collection.curr_call = 0;
	__open_collection.all_calls[__open_collection.curr_call].timestamp = (int)time(NULL);

	if (orig_open == NULL)
	{
		pthread_mutex_lock(&open_mutex);
		if (open_catched == false)
		{
			orig_open = dlsym(RTLD_NEXT, "open");
			open_catched = true;
		}
		pthread_mutex_unlock(&open_mutex);
	}

	va_list uk_arg;
	va_start(uk_arg, flag);
	int mode_ = va_arg(uk_arg, int);
	va_end(uk_arg);

	__open_collection.all_calls[__open_collection.curr_call].name = name;
	__open_collection.all_calls[__open_collection.curr_call].flag = flag;

	int fd;
	if (check_mode(mode_) == true)
	{
		__open_collection.all_calls[__open_collection.curr_call].mode = mode_;
		fd = orig_open(name, flag, mode_);
	}
	else 
	{
		fd = orig_open(name, flag);
	}

	__open_collection.all_calls[__open_collection.curr_call].fd = fd;
	__open_collection.curr_call++;

}
