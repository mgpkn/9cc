CC = gcc
CFLAGS=-std=c11 -g -fno-common -static

SRCS=$(wildcard *.c)
SRC_COMMON=common/common.c
OBJ_COMMON=$(SRC_COMMON:.c=.o)
OBJS=$(SRCS:.c=.o)

TEST_SRCS=$(wildcard test/*.c)
TEST_PRE_SRCS=$(TEST_SRCS:.c=.pc)
TEST_ASSMBLES=$(TEST_SRCS:.c=.s)
TESTS=$(TEST_SRCS:.c=.exe)

.PRECIOUS: $(TEST_PRE_SRCS) $(TEST_ASSMBLES)

9cc: $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

$(OBJS): 9cc.h

$(OBJ_COMMON):$(SRC_COMMON)
	$(CC) -c $^ -o $@


test/%.pc : $(TEST_SRCS)
	$(CC) -o- -E -P -C test/$*.c > $@

test/%.s : $(TEST_PRE_SRCS) 9cc
	./9cc test/$*.pc > $@

test/%.exe:$(OBJ_COMMON) $(TEST_ASSMBLES)
	$(CC) -o $@ test/$*.s $(OBJ_COMMON)

test: $(TESTS)
	for i in $$(echo $^ | tr ' ' '\n' | sort); do echo $$i; ./$$i || exit 1; echo; done
	test/driver.sh

clean:
	rm -rf 9cc common/*.o test/*.pc test/*.s test/*.exe
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
