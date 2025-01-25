#ifndef __DMABUF_TEST_H__
#define __DMABUF_TEST_H__
#include <assert.h>
#include <stdbool.h>

#define EXPECT_EQ(left, right)  (left == right)

struct unit_case {
    int (*run_case)(void *data);
    const char *name;
};

struct unit_suite {
    const char name[256];
    struct unit_case *test_cases;
};

#define UNIT_CASE(test_name) \
    { .run_case = test_name, .name = #test_name}

void run_unit_tests(struct unit_suite *suite) {
    struct unit_case *test_case;
    unsigned int total = 0, passed = 0;
    for (test_case = suite->test_cases; test_case->run_case; test_case++, total++) {
	if (test_case->run_case(NULL) == true)
            printf("Passed %s\n", test_case->name), passed++;
        else
            printf("Failed %s\n", test_case->name);
	}

    printf("Passed %u/%u in %s\n", passed, total, suite->name);
}

#endif /* __DMABUF_TEST_H__ */
