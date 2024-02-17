#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>

typedef struct pollfd t_pollfd;
#define POLLFD_COUNT 1
#define BUF_SIZE 1024
#define POLL_TIMEOUT 2500

#define IS_POLLIN(pollfd) ((pollfd).revents & POLLIN)

static ssize_t _process_fd(int fd)
{
	char buf[BUF_SIZE];
	ssize_t n = read(fd, buf, sizeof(buf));
	if (n < 0) {
		const char *err = strerror(errno);
		std::cerr << "Error: read failed: " << err << std::endl;
		return n;
	}
	if (n == 0) {
		std::cout << "EOF" << std::endl;
		return 0;
	}
	std::cout.write(buf, n);
	return n;
}

int main()
{
	int fd = open("/dev/stdin", O_RDONLY);
	if (fd < 0) {
		const char *err = strerror(errno);
		std::cerr << "Error: open failed: " << err << std::endl;
		return 1;
	}

	t_pollfd fds[POLLFD_COUNT];
	fds[0].fd = fd;
	fds[0].events = POLLIN;

	ssize_t exit_status;
	while (true) {
		int poll_result = poll(fds, POLLFD_COUNT, POLL_TIMEOUT);
		if (poll_result < 0) {
			const char *err = strerror(errno);
			std::cerr << "Error: poll failed: " << err << std::endl;
			return 1;
		}
		if (poll_result == 0) {
			std::cout << "Timeout" << std::endl;
			continue;
		}

		for (size_t i = 0; i < POLLFD_COUNT; i++) {
			if (IS_POLLIN(fds[i])) {
				exit_status = _process_fd(fds[i].fd);
				if (exit_status <= 0) {
					break;
				}
			}
		}
		if (exit_status <= 0) {
			break;
		}
	}

	close(fd);

	return exit_status < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
