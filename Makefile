#
# Makefile
#
# Hack Assembler
# Unit 6 Project
#

# compiler to use
CC = gcc

# flags to pass compiler
CFLAGS = -ggdb3 -O0 -std=c99 -Wall -Werror

# name for executable
EXE = asm

# space-separated list of header files
HDRS = asm.h

# space-separated list of source files
SRCS = asm.c parse.c

# automatically generated list of object files
OBJS = $(SRCS:.c=.o)


# default target
$(EXE): $(OBJS) $(HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# dependencies 
$(OBJS): $(HDRS) Makefile

# housekeeping
clean:
	rm -f core $(EXE) *.o
