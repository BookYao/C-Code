

CC = gcc

SRCDIR = ./src
INCFLAGS = -I ./include
CFLAGS = -g -Wall -std=c99

LIB = configs.a
all: $(LIB)

SRC = $(SRCDIR)/syscfg.c
OBJS = syscfg.o

$(OBJS): $(SRC)
	$(CC) -c $(CFLAGS) $(INCFLAGS) -o $@ $?

$(LIB):$(OBJS)
	ar rvs $@ $?
	rm $(OBJS)

.PHONY:clean
clean:
	rm -f $(OBJS) $(LIB)

