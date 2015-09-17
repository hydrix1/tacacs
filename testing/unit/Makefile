#
# Makefile for TACACS+ unit tests
#

ALL = dummy empty \
      string_append \

all : $(ALL)
.PHONY : all clean

MODIFIED = ../../modified
MAVIS = $(MODIFIED)/mavis
MISC  = $(MODIFIED)/misc

WRAP_SPAWND_MAIN_H = wrap_spawnd_main.h $(MAVIS)/spawnd_main.c

UNITY = unity.o
WRAP_SPAWND_MAIN = wrap_fixup.o memops.o

#settings
LINK = gcc -lsocket
CFLAGS = -I$(MODIFIED)

#rules
%:	%.o
	$(LINK) -o $@ $^

# %.o:	%.c
	# $(COMP) -o $@ $<

#dependencies
dummy.o:		dummy.c
unity.o:		unity.c unity.h
empty.o:		empty.c unity.h
memops.o:		$(MISC)/memops.c
wrap_fixup.o:		wrap_fixup.c
string_append.o:	string_append.c unity.h $(WRAP_SPAWND_MAIN_H)

dummy:			dummy.o
empty:			empty.o $(UNITY)
string_append:		string_append.o $(UNITY) $(WRAP_SPAWND_MAIN)

clean:
	rm -fr $(ALL) *.o

