#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

int dmabuf_test_ioctl() {
	int exporter_fd;
	int dmabuf_exported_fd = 0;

	exporter_fd = open("/dev/exporter", O_RDONLY);
	if (exporter_fd < 0) {
		printf("open /dev/exporter failed, %s\n", strerror(errno));
		return -1;
	}

	ioctl(exporter_fd, 0, &dmabuf_exported_fd);
	close(exporter_fd);

	char *str = mmap(NULL, 4096, PROT_READ, MAP_SHARED, dmabuf_exported_fd, 0);
	if (str == MAP_FAILED) {
		printf("mmap dmabuf failed: %s\n", strerror(errno));
		return -errno;
	}

	printf("read from dmabuf mmap: %s\n", str);
	close(dmabuf_exported_fd);

	return 0;
}

int dmabuf_test_mmap() {
	int exporter_fd;

	exporter_fd = open("/dev/exporter", O_RDONLY);
	if (exporter_fd < 0) {
		printf("open /dev/exporter failed, %s\n", strerror(errno));
		return -1;
	}

	char *str = mmap(NULL, 4096, PROT_READ, MAP_SHARED, exporter_fd, 0);
	if (str == MAP_FAILED) {
		printf("mmap /dev/exporter failed: %s\n", strerror(errno));
		return -errno;
	}

	printf("read from /dev/exporter mmap: %s\n", str);
	close(exporter_fd);

	return 0;
}

int main(int argc, char *argv[])
{
	dmabuf_test_ioctl();
	dmabuf_test_mmap();

	return 0;
}