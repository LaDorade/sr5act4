#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/timerfd.h>

void emit_msg()
{
	printf("Emit from pid %zu: bijour\n", getpid());
}

// write the message to stderr
void on_receive_msg()
{
	ssize_t ret_read;
	char buff[100];
	memset(buff, 0, sizeof buff);
	ret_read = read(0, buff, 99);
	if (ret_read < 0) {
		fprintf(stderr, "Error while reading: %s\n", strerror(errno));
		exit(1);
	}
	fprintf(stderr, "Message received to pid: %zu: %s", getpid(), buff);
}


int main(void)
{
	int     ret_poll;

	// setup a timer as a file descriptor (to poll it)
	int timer_fd = timerfd_create(CLOCK_REALTIME, 0);
	if (timer_fd < 0){
        fprintf(stderr, "Error timer_create: %s\n", strerror(errno));
        exit(-1);
    }
	struct itimerspec timerspec = {
		.it_value.tv_sec     = 1, // first wait
		.it_value.tv_nsec    = 0,
		.it_interval.tv_sec  = 1, // interval
		.it_interval.tv_nsec = 0,
	};
	timerfd_settime(timer_fd, 0, &timerspec, NULL);

	// fd that we want ot watch
	struct pollfd input[2] = {
		{fd: 0, 	   		events: POLLIN			},
		{fd: timer_fd, 		events: POLLIN | POLLOUT},
	};

	fprintf(stderr, "Starting polling\n");
	char dummyBuf[8]; // used to flush the timer
	while(1) {
		ret_poll = poll(input, 2, -1);

		if (ret_poll < 0)  // error
		{
			fprintf(stderr, "Error while polling: %s\n", strerror(errno));
			exit(1);
		} else {

			if (input[0].revents & POLLIN) // write on stdin
			{
				on_receive_msg();
			}
			if (input[1].revents & POLLIN) // timer ends -> write to stdout
			{
				read(timer_fd, dummyBuf, 8); // read timer to restart the count
				emit_msg();
			}
		}
		fflush(stdout);
	}
}
