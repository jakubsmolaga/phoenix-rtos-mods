#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/minmax.h>
#include <sys/threads.h>

// Compare pids of two threads (used for sorting)
int compare_pids(const void *a, const void *b)
{
	threadinfo_t *thread1 = (threadinfo_t *)(a);
	threadinfo_t *thread2 = (threadinfo_t *)(b);
	int pid1 = (int)(thread1->pid);
	int pid2 = (int)(thread2->pid);
	return pid1 - pid2;
}

// Print information about usage of this command
void highest_cpu_time_usage(char *cmd)
{
	printf("Finds the process with highest cpu time,\n");
	printf("and priority in range <low; high>\n\n");
	printf("Usage: %s <low> <high>\n", cmd);
	printf("    <low>    - lowest priority to consider\n");
	printf("    <high>   - highest priority to consider\n");
}

int main(int argc, char **argv)
{
	// Check if number of args is correct
	if (argc != 3) {
		highest_cpu_time_usage(argv[0]);
		return EXIT_FAILURE;
	}

	// Convert arguments to integers
	int low, high;
	if (sscanf(argv[1], "%d", &low) != 1) {
		printf("highest-cpu-time: arguments must be integers\n");
		return EXIT_FAILURE;
	}
	if (sscanf(argv[2], "%d", &high) != 1) {
		printf("highest-cpu-time: arguments must be integers\n");
		return EXIT_FAILURE;
	}

	// Make sure arguments are valid
	if (low > high) {
		printf("highest-cpu-time: provided range is invalid\n");
		printf("highest-cpu-time: %d > %d\n", low, high);
		return EXIT_FAILURE;
	}

	// Try to allocate memory for the threads list
	int n = 32;
	threadinfo_t *info = malloc(n * sizeof(threadinfo_t));
	if (info == NULL) {
		fprintf(stderr, "highest-cpu-time: out of memory\n");
		return -ENOMEM;
	}

	// Keep reallocating until you have enough memory
	int thread_count = threadsinfo(n, info);
	while (thread_count >= n) {
		n *= 2;
		threadinfo_t *tmp = realloc(info, n * sizeof(threadinfo_t));
		if (tmp == NULL) {
			fprintf(stderr, "highest-cpu-time: out of memory\n");
			free(info);
			return -ENOMEM;
		}
	}

	// Sort threads by pids
	qsort(info, thread_count, sizeof(threadinfo_t), compare_pids);

	// Find the process with the highest cpu time
	time_t best_cpu_time = 0;
	pid_t best_pid;
	int best_priority;
	int found = 0;
	int i = 0;
	for (i = 0; i < thread_count; i++) {
		// Sum cpu times of all threads within the same process
		// And find the thread with lowest priority
		pid_t current_pid = info[i].pid;
		time_t total_cpu_time = 0;
		int priority = info[i].priority;
		int j = i;
		while (info[j].pid == current_pid && j < thread_count) {
			total_cpu_time += info[j].cpuTime;
			priority = min(priority, info[j].priority);
			j++;
		}
		// Update the record
		if (total_cpu_time > best_cpu_time && priority >= low && priority <= high) {
			best_cpu_time = total_cpu_time;
			best_pid = current_pid;
			best_priority = priority;
			found = 1;
		}
		// Keep iterating
		i = j;
	}

	// Release the memory
	free(info);

	// Abort if nothing was found
	if (!found) {
		printf("highest-cpu-time: no processes with priority in range <%d; %d>\n", low, high);
		return EOK;
	}

	// Print results
	unsigned int seconds = best_cpu_time / 1000000;
	unsigned int miliseconds = (best_cpu_time / 1000) % 1000;
	printf("PID:      %d\n", best_pid);
	printf("CPU_TIME: %u.%03us\n", seconds, miliseconds);
	printf("PRIORITY: %d\n", best_priority);

	return EOK;
}
