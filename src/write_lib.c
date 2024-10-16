#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

char file_write [] = "write.out";

typedef size_t (*orig_write_f_type)(int, void*, size_t);

orig_write_f_type orig_write = NULL;

bool write_catched = false;

pthread_mutex_t write_mutex;

struct write_call {
	unsigned long int timestamp;
	int fd;
	void* buffer_ptr;
	size_t count;
	size_t bytes_written;
};

#define MAX_CALLS 256

static thread_local struct __write_array {
	struct write_call all_calls[MAX_CALLS];
	int curr_call;
} __write_collection;

void dump_write(FILE* file, struct write_call* call)
{
        fprintf(file, "%lu %d %lu %lu %lu", call->timestamp, call->fd, call->buffer_ptr,
						call->count, call->bytes_written);
}

size_t write(int fd, void *buffer, size_t count)
{
	__write_collection.curr_call = 0;
	__write_collection.all_calls[__write_collection.curr_call].timestamp = (unsigned long int)time(NULL);
	if (orig_write == NULL) {
		pthread_mutex_lock(&write_mutex);
		if (write_catched == false)
		{
			orig_write = dlsym(RTLD_NEXT, "write");
			write_catched = true;
		}
		pthread_mutex_unlock(&write_mutex);
	}

	__write_collection.all_calls[__write_collection.curr_call].fd = fd;
	__write_collection.all_calls[__write_collection.curr_call].buffer_ptr = buffer;
	__write_collection.all_calls[__write_collection.curr_call].count = count;

	size_t bytes_written;
	bytes_written = orig_write(fd, buffer, count);

	__write_collection.all_calls[__write_collection.curr_call].bytes_written = bytes_written;
	__write_collection.curr_call++;

	if (__write_collection.curr_call == MAX_CALLS)
        {
                pthread_mutex_lock(&write_mutex);

                FILE *f = fopen(file_write, "w");
                for (int i = 0; i < MAX_CALLS; ++i)
                {
                        dump_write(f, &__write_collection.all_calls[i]);
                }

                pthread_mutex_unlock(&write_mutex);
                __write_collection.curr_call = 0;
        }

	return bytes_written;
}
