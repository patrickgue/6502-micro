CFLAGS=-I../../include/Z -I../../include/6502 -g -Wall


HELPER_TEST_TARGET=./helper_test
HELPER_TEST_SRCS=./helper_test.c ../helper.c
HELPER_TEST_OBJS=$(HELPER_TEST_SRCS:.c=.o)

test:$(HELPER_TEST_TARGET)
	$(HELPER_TEST_TARGET)


$(HELPER_TEST_TARGET):$(HELPER_TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(HELPER_TEST_TARGET) $(HELPER_TEST_OBJS)