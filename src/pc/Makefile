CFLAGS=-I../ -g -Wall


TARGET=pc
SRCS=pc.c token.c ../helper.c
OBJS=$(SRCS:.c=.o)

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o:%.c
	$(CC) -c -o $@ $^ $(CFLAGS)


clean:
	rm -f $(TARGET) $(OBJS)