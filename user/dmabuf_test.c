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
#include "dmabuf_test.h"

static int dmabuf_test_exporter_ioctl(void) {
	int fd;
	int dmabuf_fd = 0;

	fd = open("/dev/exporter", O_RDONLY);
	if (fd < 0) {
		printf("open /dev/exporter failed, %s\n", strerror(errno));
		return -1;
	}

	ioctl(fd, 0, &dmabuf_fd);
	close(fd);

	char *str = mmap(NULL, 4096, PROT_READ, MAP_SHARED, dmabuf_fd, 0);
	if (str == MAP_FAILED) {
		printf("mmap dmabuf failed: %s\n", strerror(errno));
		return -errno;
	}

	printf("read from dmabuf mmap: %s\n", str);
	munmap(str, 4096);
	close(dmabuf_fd);

	return 0;
}

static int dmabuf_test_importer_ioctl(void) {
	int fd;
	int dmabuf_fd = 0;

	fd = open("/dev/exporter", O_RDONLY);
	ioctl(fd, 0, &dmabuf_fd);
	close(fd);

	fd = open("/dev/importer", O_RDONLY);
	ioctl(fd, 0, &dmabuf_fd);
	close(fd);

	return 0;
}

static int dmabuf_test_mmap(void) {
	int fd;

	fd = open("/dev/exporter", O_RDONLY);
	if (fd < 0) {
		printf("open /dev/exporter failed, %s\n", strerror(errno));
		return -1;
	}

	char *str = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);
	if (str == MAP_FAILED) {
		printf("mmap /dev/exporter failed: %s\n", strerror(errno));
		return -errno;
	}

	printf("read from /dev/exporter mmap: %s\n", str);
	munmap(str, 4096);
	close(fd);

	return 0;
}

static int unit_dmabuf_test_exporter_ioctl(void *data) {
	return EXPECT_EQ(dmabuf_test_exporter_ioctl(), 0);
}

static int unit_dmabuf_test_importer_ioctl(void *data) {
	return EXPECT_EQ(dmabuf_test_importer_ioctl(), 0);
}

static int unit_dmabuf_test_mmap(void *data) {
	return EXPECT_EQ(dmabuf_test_mmap(), 0);
}

static struct unit_case unit_test_cases[] = {
	UNIT_CASE(unit_dmabuf_test_exporter_ioctl),
	UNIT_CASE(unit_dmabuf_test_importer_ioctl),
	UNIT_CASE(unit_dmabuf_test_mmap),
	{},
};

static struct unit_suite unit_test_suite = {
	.name = "dmabuf user unit test",
	.test_cases = unit_test_cases,
};

int main(int argc, char *argv[])
{
	run_unit_tests(&unit_test_suite);

	return 0;
}
