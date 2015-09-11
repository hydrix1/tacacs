#
# Makefile for TACACS+ unit tests
#

ALL = dummy empty
all : $(ALL)
.PHONY : all clean

# .SUFFIXES:
# .SUFFIXES: .c .o .h

#settings
# CC = gcc
# COMP = $(CC) -c
# LINK = $(CC)

UNITY = unity.o

#rules
# %:	%.o
	# $(LINK) -o $@ $^

# %.o:	%.c
	# $(COMP) -o $@ $<

#dependencies
dummy.o:	dummy.c
empty.o:	empty.c unity.h
unity.o:	unity.c unity.h

dummy:	dummy.o
empty:	empty.o $(UNITY)

clean:
	rm -fr $(ALL) *.o

