#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <highest_cpu_time.h>
 
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

	int pid = 0;
	unsigned long long cpu_time;
	highest_cpu_time(low, high, &pid, &cpu_time);

	// // Abort if nothing was found
	// if (!found) {
	// 	printf("highest-cpu-time: no processes with priority in range <%d; %d>\n", low, high);
	// 	return EOK;
	// }

	// Print results
	unsigned int seconds = cpu_time / 1000000;
	unsigned int miliseconds = (cpu_time / 1000) % 1000;
	printf("PID:      %d\n", pid);
	printf("CPU_TIME: %u.%03us\n", seconds, miliseconds);

	return 0;
}
