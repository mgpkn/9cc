CFLAGS=-std=c11 -g -fno-common

SRCS=$(wildcard *.c)
SRC_COMMON=common/common.c
OBJ_COMMON=$(SRC_COMMON:.c=.o)
OBJS=$(SRCS:.c=.o)

TEST_SRCS=$(wildcard test/*.c)
TEST_PRI_SRCS=$(TEST_SRCS:.c=.cc)
TEST_ASSMBLES=$(TEST_SRCS:.c=.s)
TESTS=$(TEST_SRCS:.c=.exe)

9cc: $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

$(OBJS): 9cc.h

$(OBJ_COMMON):$(SRC_COMMON)
	$(CC) -c $^ -o $@

test/%.cc:test/%.c
	$(CC) -o- -E -P -C test/$*.c > $@

test/%.exe: 9cc test/%.cc $(OBJ_COMMON) 
	./9cc test/$*.cc > test/$*.s
	$(CC) -o $@ test/$*.s $(OBJ_COMMON)

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done
	test/driver.sh

otest: 9cc
	./test.sh

clean:
	rm -rf 9cc common/*.o $(TESTS) $(TEST_ASSMBLES) $(TEST_PRI_SRCS)
	find * -type f '(' -name '*~' -o -name '*.o' ')' -exec rm {} ';'

params:
#	echo $(CC)
#	echo $(LDFLAGS)	
#	echo $(CFLAGS)
	echo $(SRCS)
	echo $(OBJS)
	echo $(TESTS)	
	echo $(SRC_COMMON)
	echo $(OBJ_COMMON)	

.PHONY: test clean params
