#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <sys/event.h>
#include <sys/time.h>
#else
#include <poll.h>
#include <sys/timerfd.h>
#endif // __APPLE__


void hang()
{
	// fprintf(stderr, "."); for (int i = 0; i < 1000000000; i++) {
	// 	// do nothing
	// 	asm("");
	// }
	// fprintf(stderr, "\n");
}

void emit_msg(char* message)
{
	hang();
	printf("Emittin msg: '%s', from pid %zu: bijour\n", message, getpid());
}

// write the message to stderr
void on_receive_msg()
{
	hang();
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


int main(int argc, char **argv)
{
	char message[128];
	if (argc >= 2) {
		strcpy(message, argv[1]);
	} else {
		strcpy(message, "basic message");
	}

	int     ret_poll;

#ifdef __APPLE__
	struct kevent changelist[2] = {
		{
			.ident = STDIN_FILENO,
			.filter = EVFILT_READ,
			.flags = EV_ADD,
			.fflags = 0,
			.data = 0,
			.udata = NULL,
		},
		{
			.ident = 4,
			.flags = EV_ADD,
			.data = 1000000,
			.filter = EVFILT_TIMER,
			.fflags = NOTE_USECONDS | EV_CLEAR,
		}
	};
	int queue = kqueue();
	assert(-1 != queue);
	int res = kevent(queue, &changelist, 2, NULL, 0, 0);
	assert(-1 != res);
	struct kevent eventlist[2] = {0};
#else
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
#endif // __APPLE

	fprintf(stderr, "Starting polling\n");
	char dummyBuf[8]; // used to flush the timer
	while(1) {
#ifdef __APPLE__
		res = kevent(queue, NULL, 0, &eventlist, 2, NULL);
		assert(-1 != res);

		for (int i = 0; i < res; i++) {
			struct kevent event = eventlist[i];
			if (event.ident == 4 && event.filter & EVFILT_TIMER) {
				emit_msg(message);
			} else if (event.ident == STDIN_FILENO) {
				on_receive_msg();
			}
		}
#else
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
				emit_msg(message);
			}
		}
#endif
		fflush(stdout);
	}
}
