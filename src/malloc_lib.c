#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

char file_malloc [] = "malloc.out";

typedef void* (*orig_malloc_f_type)(size_t);

orig_malloc_f_type orig_malloc = NULL;

bool malloc_catched = false;

pthread_mutex_t malloc_mutex;

struct malloc_call {
	unsigned long int timestamp;
	size_t bytes_req;
	void* new_mem_p;
};

#define MAX_CALLS 256

static thread_local struct __malloc_array {
	struct malloc_call all_calls[MAX_CALLS];
	int curr_call;
} __malloc_collection;

void dump_malloc(FILE* file, struct malloc_call* call)
{
	fprintf(file, "%lu %lu %lu", call->timestamp, call->bytes_req, call->new_mem_p);
}

void *malloc(size_t sizemem)
{
	__malloc_collection.curr_call = 0;
	__malloc_collection.all_calls[__malloc_collection.curr_call].timestamp = (unsigned long int)time(NULL);
	if (orig_malloc == NULL) {
		pthread_mutex_lock(&malloc_mutex);
		if (malloc_catched == false)
		{
			orig_malloc = dlsym(RTLD_NEXT, "malloc");
			malloc_catched = true;
		}
		pthread_mutex_unlock(&malloc_mutex);
	}

	__malloc_collection.all_calls[__malloc_collection.curr_call].bytes_req = sizemem;

	void* new_mem_p;
	new_mem_p = orig_malloc(sizemem);

	__malloc_collection.all_calls[__malloc_collection.curr_call].new_mem_p = new_mem_p;
	__malloc_collection.curr_call++;

	if (__malloc_collection.curr_call == MAX_CALLS) {
                pthread_mutex_lock(&malloc_mutex);
		FILE *f = fopen(file_malloc, "w");
		for (int i = 0; i < MAX_CALLS; ++i)                                                           
                {
                        dump_malloc(f, &__malloc_collection.all_calls[i]);
                }
                pthread_mutex_unlock(&malloc_mutex);
                __malloc_collection.curr_call = 0;
        }

	return new_mem_p;
}
