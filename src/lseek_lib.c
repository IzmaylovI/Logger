#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

char lseek_free [] = "lseek.out";

typedef off_t (*orig_lseek_f_type)(int, off_t, int);

orig_lseek_f_type orig_lseek = NULL;

bool lseek_catched = false;

pthread_mutex_t lseek_mutex;

struct lseek_call {
	unsigned long int timestamp;
	off_t res_offset;
	int fd;
	off_t offset;
	int whence;
};

#define MAX_CALLS 256

static thread_local struct __lseek_array {
	struct lseek_call all_calls[MAX_CALLS];
	int curr_call;
} __lseek_collection;

void dump_lseek(FILE* file, struct lseek_call* call)
{
        fprintf(file, "%lu %ld %d %ld %d", call->timestamp, call->res_offset, call->fd,
						call->offset, call->whence);
}

off_t lseek(int fd, off_t offset, int whence)
{
	__lseek_collection.curr_call = 0;
	__lseek_collection.all_calls[__lseek_collection.curr_call].timestamp = (unsigned long int)time(NULL);
	if (orig_lseek == NULL)
	{
		pthread_mutex_lock(&lseek_mutex);
		if (lseek_catched == false)
		{
			orig_lseek = dlsym(RTLD_NEXT, "lseek");
			lseek_catched = true;
		}
		pthread_mutex_unlock(&lseek_mutex);
	}

	__lseek_collection.all_calls[__lseek_collection.curr_call].fd = fd;
	__lseek_collection.all_calls[__lseek_collection.curr_call].offset = offset;
	__lseek_collection.all_calls[__lseek_collection.curr_call].whence = whence;

	int res_offset;
	res_offset = orig_lseek(fd, offset, whence);

	__lseek_collection.all_calls[__lseek_collection.curr_call].res_offset = res_offset;
	__lseek_collection.curr_call++;

	if (__lseek_collection.curr_call == MAX_CALLS)
        {
                pthread_mutex_lock(&lseek_mutex);

                FILE *f = fopen(lseek_free, "w");
                for (int i = 0; i < MAX_CALLS; ++i)
                {
                        dump_lseek(f, &__lseek_collection.all_calls[i]);
                }

                pthread_mutex_unlock(&lseek_mutex);
                __lseek_collection.curr_call = 0;
        }

	return	res_offset;
}
