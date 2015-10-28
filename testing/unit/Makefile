#
# Makefile for TACACS+ unit tests
#

ALL = dummy empty \
      string_append \
      get_missing_argument \
      get_omitted_argument \
      get_required_argument \
      get_optional_argument \
      get_next_option \
      parse_listen_subopts \
      parse_cli_listen \
      set_password \
      parse_level_subopts \
      parse_host_subopts \
      parse_cli_debug \
      parse_cli_host \
      parse_cmd_subopts \
      parse_group_subopts \
      parse_cli_group \
      get_user_shell \
      parse_user_subopts \
      parse_cli_user \
      generate_more \
      generate_listen_request \
      generate_spawnd_config \
      generate_level_config \
      generate_level_set_config \
      generate_host_config \
      generate_access_config \
      generate_cmd_config \
      generate_service_config \
      generate_group_config \
      generate_user_config \
      generate_tacplus_config \
      generate_cli_config \

all : $(ALL)
.PHONY : all clean

MODIFIED = ../../modified
MAVIS = $(MODIFIED)/mavis
MISC  = $(MODIFIED)/misc

UNITY = unity.o
WRAP_SPAWND_MAIN = wrap_fixup.o memops.o getline.o

#settings
UNAME = $(shell uname)
ifeq ($(UNAME), SunOS)
  LINK = gcc -lsocket
else
  LINK = gcc
endif
CFLAGS = -I$(MODIFIED)

#rules
%:	%.o
	$(LINK) -o $@ $^

%.o:	$(MISC)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

#dependencies
unity.o:			unity.c unity.h
getline.o:			$(MISC)/getline.c
memops.o:			$(MISC)/memops.c
wrap_fixup.o:			wrap_fixup.c wrap_spawnd_main.h $(MAVIS)/spawnd_main.c
dummy:				dummy.o
empty:				empty.o unity.h $(UNITY)
string_append:			string_append.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_missing_argument:		get_missing_argument.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_omitted_argument:		get_omitted_argument.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_required_argument:		get_required_argument.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_optional_argument:		get_optional_argument.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_next_option:		get_next_option.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_listen_subopts:		parse_listen_subopts.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_listen:		parse_cli_listen.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
set_password:			set_password.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_level_subopts:		parse_level_subopts.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_host_subopts:		parse_host_subopts.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_debug:		parse_cli_debug.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_host:			parse_cli_host.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cmd_subopts:		parse_cmd_subopts.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_group_subopts:		parse_group_subopts.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_group:		parse_cli_group.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_user_shell:			get_user_shell.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_user_subopts:		parse_user_subopts.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_user:			parse_cli_user.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_more:			generate_more.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_listen_request:	generate_listen_request.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_spawnd_config:		generate_spawnd_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_level_config:		generate_level_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_level_set_config:	generate_level_set_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_host_config:		generate_host_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_access_config:		generate_access_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_cmd_config:		generate_cmd_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_service_config:	generate_service_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_group_config:		generate_group_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_user_config:		generate_user_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_tacplus_config:	generate_tacplus_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_cli_config:		generate_cli_config.o unity.h $(WRAP_SPAWND_MAIN) $(UNITY)

clean:
	rm -fr $(ALL) *.o

