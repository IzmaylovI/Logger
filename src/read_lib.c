#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

char file_read[] = "read.out";

typedef size_t (*orig_read_f_type)(int, void*, size_t);

orig_read_f_type orig_read = NULL;

bool read_catched = false;

pthread_mutex_t read_mutex;

struct read_call {
	unsigned long int timestamp;
	int fd;
	void* buffer_ptr;
	size_t count;
	size_t bytes_read;
};

#define MAX_CALLS 256

static thread_local struct __read_array {
	struct read_call all_calls[MAX_CALLS];
	int curr_call;
} __read_collection;

void dump_read(FILE* file, struct read_call* call)
{
        fprintf(file, "%lu %d %lu %lu %lu", call->timestamp, call->fd, call->buffer_ptr,
						call->count, call->bytes_read);
}

size_t read(int fd, void *buffer, size_t count)
{
	__read_collection.curr_call = 0;
	__read_collection.all_calls[__read_collection.curr_call].timestamp = (unsigned long int)time(NULL);
	if (orig_read == NULL) {
		pthread_mutex_lock(&read_mutex);
		if (read_catched == false)
		{
			orig_read = dlsym(RTLD_NEXT, "read");
			read_catched = true;
		}
		pthread_mutex_unlock(&read_mutex);
	}

	__read_collection.all_calls[__read_collection.curr_call].fd = fd;
	__read_collection.all_calls[__read_collection.curr_call].buffer_ptr = buffer;
	__read_collection.all_calls[__read_collection.curr_call].count = count;

	size_t bytes_read;
	bytes_read = orig_read(fd, buffer, count);

	__read_collection.all_calls[__read_collection.curr_call].bytes_read = bytes_read;
	__read_collection.curr_call++;

	if (__read_collection.curr_call == MAX_CALLS)
        {
                pthread_mutex_lock(&read_mutex);

                FILE *f = fopen(file_read, "w");
                for (int i = 0; i < MAX_CALLS; ++i)
                {
                        dump_read(f, &__read_collection.all_calls[i]);
                }

                pthread_mutex_unlock(&read_mutex);
                __read_collection.curr_call = 0;
        }

	return bytes_read;
}
