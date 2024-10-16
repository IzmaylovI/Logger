#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

char file_free [] = "free.out";

typedef void (*orig_free_f_type)(void*);

orig_free_f_type orig_free = NULL;

bool free_catched = false;

pthread_mutex_t free_mutex;

struct free_call {
	unsigned long int timestamp;
	void *mem_ptr;
};

#define MAX_CALLS 256

static thread_local struct __free_array {
    struct free_call all_calls[MAX_CALLS];
	int curr_call;
} __free_collection;

void dump_free(FILE* file, struct free_call* call)
{
        fprintf(file, "%lu %lu", call->timestamp, call->mem_ptr);
}

void free(void *ptr)
{
	__free_collection.curr_call = 0;
	__free_collection.all_calls[__free_collection.curr_call].timestamp = (unsigned long int)time(NULL);
	if (orig_free == NULL)
	{
		pthread_mutex_lock(&free_mutex);
		if (free_catched == false)
		{
			orig_free = dlsym(RTLD_NEXT, "realloc");
			free_catched = true;
		}
		pthread_mutex_unlock(&free_mutex);
	}

	__free_collection.all_calls[__free_collection.curr_call].mem_ptr = ptr;
	__free_collection.curr_call++;

	if (__free_collection.curr_call == MAX_CALLS)
        {
                pthread_mutex_lock(&free_mutex);

                FILE *f = fopen(file_free, "w");
                for (int i = 0; i < MAX_CALLS; ++i)
                {
                        dump_close(f, &__free_collection.all_calls[i]);
                }

                pthread_mutex_unlock(&free_mutex);
                __free_collection.curr_call = 0;
        }

	orig_free(ptr);
}
