CFLAGS=-std=c11 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -c foo/foo.c 
	$(CC) -o 9cc $(OBJS) $(CFLAGS) foo.o

$(OBJS): 9cc.h

test: 9cc
	./test.sh

params:
	echo $(CC)
	echo $(CFLAGS)
	echo $(SRCS)
	echo $(OBJS)

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean params
