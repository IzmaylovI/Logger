#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

char file_realloc[] = "realloc.out";

typedef void* (*orig_realloc_f_type)(void*, size_t);

orig_realloc_f_type orig_realloc = NULL;

bool realloc_catched = false;

pthread_mutex_t realloc_mutex;

struct realloc_call {
	unsigned long int timestamp;
	void *curr_mem_ptr;
	size_t bytes_req;
	void *new_mem_ptr;
};

#define MAX_CALLS 256

static thread_local struct __realloc_array {
    struct realloc_call all_calls[MAX_CALLS];
	int curr_call;
} __realloc_collection;

void dump_realloc(FILE* file, struct realloc_call* call)
{
        fprintf(file, "%lu %lu %lu %lu", call->timestamp, call->curr_mem_ptr, call->bytes_req, call->new_mem_ptr);
}

void *realloc(void *ptr, size_t size)
{
	__realloc_collection.curr_call = 0;
	__realloc_collection.all_calls[__realloc_collection.curr_call].timestamp = (unsigned long int)time(NULL);
	if (orig_realloc == NULL)
	{
		pthread_mutex_lock(&realloc_mutex);
		if (realloc_catched == false)
		{
			orig_realloc = dlsym(RTLD_NEXT, "realloc");
			realloc_catched = true;
		}
		pthread_mutex_unlock(&realloc_mutex);
	}

	__realloc_collection.all_calls[__realloc_collection.curr_call].curr_mem_ptr = ptr;
	__realloc_collection.all_calls[__realloc_collection.curr_call].bytes_req = size;

	void *new_mem_ptr;
	new_mem_ptr = orig_realloc(ptr, size);

	__realloc_collection.all_calls[__realloc_collection.curr_call].curr_mem_ptr = new_mem_ptr;
	__realloc_collection.curr_call++;

	if (__realloc_collection.curr_call == MAX_CALLS)
        {
                pthread_mutex_lock(&realloc_mutex);

                FILE *f = fopen(file_realloc, "w");
                for (int i = 0; i < MAX_CALLS; ++i)
                {
                        dump_realloc(f, &__realloc_collection.all_calls[i]);
                }

                pthread_mutex_unlock(&realloc_mutex);
                __realloc_collection.curr_call = 0;
        }

	return new_mem_ptr;
}
