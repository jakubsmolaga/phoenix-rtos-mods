#include <stdio.h>
#include <stdlib.h>
#include <ch_sched_group.h>
#include <sys/threads.h>
#include <errno.h>

int main(void)
{
	ch_sched_group(1);

	// Allocate memory for the threads list
	int n = 32;
	threadinfo_t *info = malloc(n * sizeof(threadinfo_t));
	if (info == NULL) {
		printf("change-sched-group: failed to allocate memory for threads list\n");
		return EXIT_FAILURE;
	}

	// Get the list of threads
	int ret = threadsinfo(n, info);
	if (ret < 0) {
		printf("change-sched-group: failed to get threads list\n");
		return EXIT_FAILURE;
	}

	// If the list is too small, try to allocate more memory
	if (ret > n) {
		n = ret;
		info = realloc(info, n * sizeof(threadinfo_t));
		if (info == NULL) {
			printf("change-sched-group: failed to allocate memory for threads list\n");
			return EXIT_FAILURE;
		}

		// Get the list of threads
		ret = threadsinfo(n, info);
		if (ret < 0) {
			printf("change-sched-group: failed to get threads list\n");
			return EXIT_FAILURE;
		}
	}

	// Find current thread id
	int tid = gettid();

	// Find current thread in the list
	for (size_t i = 0; i < n; i++) {
		if (info[i].tid == tid) {
			printf("Found the thread!\n");
		}
	}

	printf("Hello, world!\n");
	return 0;
};