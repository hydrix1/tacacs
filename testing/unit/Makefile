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
WRAP_SPAWND_MAIN = wrap_fixup.o memops.o getline.o wrap_spawnd_main.h

#settings
#LINK = gcc -lsocket
LINK = gcc
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
dummy:				dummy.c
empty:				empty.c unity.h $(UNITY)
string_append:			string_append.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_missing_argument:		get_missing_argument.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_omitted_argument:		get_omitted_argument.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_required_argument:		get_required_argument.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_optional_argument:		get_optional_argument.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_next_option:		get_next_option.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_listen_subopts:		parse_listen_subopts.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_listen:		parse_cli_listen.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
set_password:			set_password.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_level_subopts:		parse_level_subopts.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_host_subopts:		parse_host_subopts.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_debug:		parse_cli_debug.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_host:			parse_cli_host.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cmd_subopts:		parse_cmd_subopts.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_group_subopts:		parse_group_subopts.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_group:		parse_cli_group.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
get_user_shell:			get_user_shell.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_user_subopts:		parse_user_subopts.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
parse_cli_user:			parse_cli_user.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_more:			generate_more.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_listen_request:	generate_listen_request.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_spawnd_config:		generate_spawnd_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_level_config:		generate_level_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_level_set_config:	generate_level_set_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_host_config:		generate_host_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_access_config:		generate_access_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_cmd_config:		generate_cmd_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_service_config:	generate_service_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_group_config:		generate_group_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_user_config:		generate_user_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_tacplus_config:	generate_tacplus_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)
generate_cli_config:		generate_cli_config.c unity.h $(WRAP_SPAWND_MAIN) $(UNITY)

clean:
	rm -fr $(ALL) *.o

