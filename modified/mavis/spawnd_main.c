/*
 * spawnd_main.c
 * (C)2000-2011 by Marc Huber <Marc.Huber@web.de>
 *
 * $Id: spawnd_main.c,v 1.90 2015/03/14 06:11:28 marc Exp marc $
 *
 */

#define __MAIN__

#include "spawnd_headers.h"
#include "misc/version.h"
#include "misc/sig_segv.h"
#include "misc/pid_write.h"
#include <signal.h>
#include <sysexits.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>

#ifdef __APPLE__
#  include <mach-o/dyld.h>
#endif
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

static const char rcsid[] __attribute__ ((used)) = "$Id: spawnd_main.c,v 1.90 2015/03/14 06:11:28 marc Exp marc $";

struct spawnd_data spawnd_data;	/* configuration data */

static void periodics(struct spawnd_context *ctx, int cur __attribute__ ((unused)))
{
    pid_t deadpid = -1;
    int status, i;

    DebugIn(DEBUG_PROC);

    io_sched_renew(ctx->io, ctx);

    while (0 < (deadpid = waitpid(-1, &status, WNOHANG))) {
	int sig = 0;

	if (!WIFEXITED(status) && WIFSIGNALED(status))
	    sig = WTERMSIG(status);

	for (i = 0; i < common_data.servers_cur && spawnd_data.server_arr[i]->pid != deadpid; i++);

	if (sig)
	    logmsg("child (pid %u) terminated abnormally (signal %d)", (u_int) deadpid, sig);
	else
	    logmsg("child (pid %u) terminated normally", (u_int) deadpid);

	if (i < common_data.servers_cur)
	    spawnd_cleanup_internal(spawnd_data.server_arr[i], spawnd_data.server_arr[i]->fn);
    }

    spawnd_cleanup_tracking();

    spawnd_process_signals();

    if (common_data.users_cur < (common_data.users_min * (common_data.servers_cur + 1))) {
	int servers_count = common_data.servers_cur;
	for (i = 0; i < common_data.servers_cur; i++)
	    if (spawnd_data.server_arr[i]->dying)
		servers_count--;

	Debug((DEBUG_PROC, "servers_cur: %d\n", common_data.servers_cur));
	Debug((DEBUG_PROC, "servers_count: %d\n", servers_count));
	Debug((DEBUG_PROC, "servers_min: %d\n", common_data.servers_min));

	for (i = 0; i < common_data.servers_cur && servers_count > common_data.servers_min; i++)
	    if (!spawnd_data.server_arr[i]->use) {
		if (!spawnd_data.server_arr[i]->dying) {
		    struct scm_data sd;
		    sd.type = SCM_MAY_DIE;
		    spawnd_data.server_arr[i]->dying = 1;
		    Debug((DEBUG_PROC, "server %d may die\n", i));
		    common_data.scm_send_msg(spawnd_data.server_arr[i]->fn, &sd, -1);
		}
		servers_count--;
	    }
    }
    DebugOut(DEBUG_PROC);
}

void get_exec_path(char **path, char *dflt)
{
    char tmp[PATH_MAX];
    ssize_t rls;

    if (strchr(dflt, '/')) {
	*path = strdup(dflt);
	return;
    }
#if defined(CTL_KERN) && defined(KERN_PROC) && defined(KERN_PROC_PATHNAME)
    {
	size_t size = sizeof(tmp);
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;
	if (!sysctl(mib, 4, tmp, &size, NULL, 0)) {
	    *path = strdup(tmp);
	    return;
	}
    }
#endif

#if defined(__APPLE__)
    {
	uint32_t size = sizeof(tmp);
	if (!_NSGetExecutablePath(tmp, &size)) {
	    *path = strdup(tmp);
	    return;
	}
    }
#endif

#if defined(__sun__)
    {
	char *p = (char *) getexecname();
	if (p) {
	    *path = strdup(p);
	    return;
	}
    }
#endif

    rls = readlink("/proc/self/exe", tmp, sizeof(tmp));
    if (rls < 0)
	rls = readlink("/proc/curproc/file", tmp, sizeof(tmp));
    if (rls < 0) {
	snprintf(tmp, sizeof(tmp), "/proc/%lu/exe", (u_long) getpid());
	rls = readlink(tmp, tmp, sizeof(tmp));
    }
    if (rls > 0) {
	tmp[rls] = 0;
	*path = strdup(tmp);
	return;
    }

    *path = strdup(dflt);
}

static char **dup_array(char **a)
{
    int i;
    char **v;

    v = a, i = 0;
    while (*v)
	v++, i++;

    v = calloc(++i, sizeof(char *));

    i = 0;
    while (*a) {
	v[i] = strdup(*a);
	a++, i++;
    }

    return v;
}

struct spawnd_context *spawnd_new_context(struct io_context *io)
{
    struct spawnd_context *c = Xcalloc(1, sizeof(struct spawnd_context));
    c->io = io;
    c->fn = -1;
    c->keepintvl = -1;
    c->keepcnt = -1;
    c->keepidle = -1;

    return c;
}

int spawnd_note_listener(sockaddr_union * sa __attribute__ ((unused)), void *data)
{
    spawnd_data.listener_arr = Xrealloc(spawnd_data.listener_arr, (spawnd_data.listeners_max + 1) * sizeof(struct spawnd_context *));
    spawnd_data.listener_arr[spawnd_data.listeners_max] = (struct spawnd_context *) data;
    memcpy(&spawnd_data.listener_arr[spawnd_data.listeners_max++]->sa, sa, sizeof(sockaddr_union));
    return 0;
}

void spawnd_bind_listener(struct spawnd_context *ctx, int cur)
{

    char buf[INET6_ADDRSTRLEN];

    DebugIn(DEBUG_NET);
    if (ctx->fn < 0) {
	io_sched_del(common_data.io, ctx, (void *) spawnd_bind_listener);

	cur = su_socket(ctx->sa.sa.sa_family, ctx->socktype, ctx->protocol);

	if (cur < 0) {
	    logerr("socket(%d, %d, %d) [%s:%d]", ctx->sa.sa.sa_family, ctx->socktype, ctx->protocol, __FILE__, __LINE__);
	    if (ctx->retry_delay)
		io_sched_add(common_data.io, ctx, (void *) spawnd_bind_listener, (time_t) ctx->retry_delay, (suseconds_t) 0);
	    DebugOut(DEBUG_NET);
	    return;
	}
#ifdef AF_UNIX
	if (ctx->sa.sa.sa_family == AF_UNIX)
	    unlink(ctx->sa.sun.sun_path);
#endif				/* AF_UNIX */

	if (su_bind(cur, &ctx->sa)) {
	    if (!ctx->logged_retry)
		logerr("bind (%s:%d)", __FILE__, __LINE__);
#ifdef AF_UNIX
	    if (ctx->sa.sa.sa_family == AF_UNIX) {
		if (!ctx->logged_retry) {
		    ctx->logged_retry = 1;
		    if (ctx->retry_delay)
			logmsg("bind to %s failed. Will retry every %d seconds.", ctx->sa.sun.sun_path, ctx->retry_delay);
		    else
			logmsg("bind to %s failed.", ctx->sa.sun.sun_path);
		}
	    } else
#endif
	    if (!ctx->logged_retry) {
		ctx->logged_retry = 1;
		if (ctx->retry_delay)
		    logmsg
			("bind to [%s]:%d failed. Will retry every %d seconds.",
			 su_ntop(&ctx->sa, buf, (socklen_t) sizeof(buf)), su_get_port(&ctx->sa), ctx->retry_delay);
		else
		    logmsg("bind to [%s]:%d failed.", su_ntop(&ctx->sa, buf, (socklen_t) sizeof(buf)), su_get_port(&ctx->sa));
	    }
	    if (ctx->retry_delay)
		io_sched_add(common_data.io, ctx, (void *) spawnd_bind_listener, (time_t) ctx->retry_delay, (suseconds_t) 0);
	    else {
		spawnd_data.bind_failures++;
		if (spawnd_data.bind_failures == spawnd_data.listeners_max) {
		    logmsg("Failed to bind to any address or port. Exiting.");
		    exit(EX_TEMPFAIL);
		}
	    }

	    Debug((DEBUG_NET, "- %s (bind error)\n", __func__));
	    close(cur);
	    return;
	}

	ctx->fn = cur;

#ifdef AF_UNIX
	if (ctx->sa.sa.sa_family == AF_UNIX) {
	    if (chown(ctx->sa.sun.sun_path, ctx->uid, ctx->gid))
		logerr("chown(%s) (%s:%d)", ctx->sa.sun.sun_path, __FILE__, __LINE__);

	    if (ctx->mode)
		if (chmod(ctx->sa.sun.sun_path, ctx->mode)) {
		    logerr("chmod(%s) (%s:%d)", ctx->sa.sun.sun_path, __FILE__, __LINE__);
		}
	}
#endif				/* AF_UNIX */
    }

    logmsg("bind to [%s]:%d succeeded%s", su_ntop(&ctx->sa, buf, (socklen_t) sizeof(buf)), su_get_port(&ctx->sa), ctx->fn ? "" : " (via inetd)");

    if (listen(ctx->fn, ctx->listen_backlog)) {
	logerr("listen (%s:%d)", __FILE__, __LINE__);
	Debug((DEBUG_NET, "- %s (listen error)\n", __func__));
	return;
    }

    ctx->is_listener = 1;

    ctx->io = common_data.io;
    io_register(common_data.io, ctx->fn, ctx);
    io_set_cb_i(common_data.io, ctx->fn, (void *) spawnd_accepted);
    io_clr_cb_o(common_data.io, ctx->fn);
    io_set_cb_e(common_data.io, ctx->fn, (void *) spawnd_cleanup_internal);
    io_set_cb_h(common_data.io, ctx->fn, (void *) spawnd_cleanup_internal);
    io_set_i(common_data.io, ctx->fn);

    DebugOut(DEBUG_NET);
}

typedef enum {
    lopt_end = 300,
    lopt_print,
    lopt_listen,
    lopt_port,
    lopt_host,
    lopt_key,
    lopt_address,
    lopt_user,
    lopt_password,
    lopt_debug,
    lopt_cmd,
    lopt_deny,
    lopt_permit,
} lopts_e;

static struct option long_opts[] =
{
    /* Long version of standard short options */
    /* "vPd:i:p:c:bf1" */
    { "help",        no_argument,       0, 'h' },
    { "version",     no_argument,       0, 'v' },
    { "check",       no_argument,       0, 'P' },
    { "degraded",    no_argument,       0, 'l' },
    { "child-id",    required_argument, 0, 'i' },
    { "debug-levek", required_argument, 0, 'd' },
    { "pid-file",    required_argument, 0, 'p' },
    { "foreground",  no_argument,       0, 'f' },
    { "background",  no_argument,       0, 'b' },
    /* CLI configuration options */
    { "print",       no_argument,       0, lopt_print },
    { "listen",      no_argument,       0, lopt_listen },
    { "port",        required_argument, 0, lopt_port },
    { "host",        optional_argument, 0, lopt_host },
    { "key",         required_argument, 0, lopt_key },
    { "address",     required_argument, 0, lopt_address },
    { "user",        required_argument, 0, lopt_user },
    { "password",    required_argument, 0, lopt_password },
    { "debug",       required_argument, 0, lopt_debug },
    { "cmd",         required_argument, 0, lopt_cmd },
    { "deny",        optional_argument, 0, lopt_deny },
    { "permit",      optional_argument, 0, lopt_permit },
    /* end marker */
    { 0, 0, 0, 0}
};

struct listen_config
{
    struct listen_config*   next_listen;
    char*                   port;
};

struct host_config
{
    struct host_config*     next_host;
    char*                   host_name;
    char*                   secret_key;
    char*                   address;
};

struct access_config
{
    struct access_config*   next_access;
    char*                   access_type;
    char*                   access_regex;
};

struct cmd_config
{
    struct cmd_config*      next_cmd;
    char*                   cmd_name;
    struct access_config*   access_list;
    struct access_config*   access_last;
};

struct service_config
{
    struct service_config*  next_service;
    char*                   svc_name;
    struct cmd_config*      cmd_list;
    struct cmd_config*      cmd_last;
};

struct user_config
{
    struct user_config*     next_user;
    char*                   username;
    char*                   password;
    struct service_config*  service_list;
    struct service_config*  service_last;
};

struct spawnd_config
{
    struct listen_config*   listen_list;
    struct listen_config*   listen_last;
};

struct tacplus_config
{
    struct host_config*     host_list;
    struct host_config*     host_last;
    struct user_config*     user_list;
    struct user_config*     user_last;
};

struct cli_config
{
    int                    used;
    int                    print;
    struct spawnd_config   spawnd;
    struct tacplus_config  tacplus;
};

extern char* optarg;
extern int optind;
static int opts_argc;
static char** opts_argv;
static const char* short_opts = ":hvPd:i:p:c:bf1";
static int opts_index;

static char* get_missing_argument(const char* prompt, va_list args)
{
    char*   result;
    size_t  space = 0;
    ssize_t count = 0;
    char*   buffer = 0;

    do
    {
	fprintf (stdout, "Missing ");
	vfprintf (stdout, prompt, args);
	fprintf (stdout, "\n>> ");
	count = getline(&buffer, &space, stdin);
	if (count < 0)
	{
	    fprintf (stderr, "EOF on stdin while reading mandatory parameter!\n");
	    exit(1);
	}
	result = buffer;
	while ((count > 0) && (result[count-1] <= ' '))
	{
	    result[--count] = '\0';
	}
	while (*result && *result <= ' ')
	{
	    ++result;
	}
    } while (*result == '\0');

    result = Xstrdup(result);
    free (buffer);

    return result;
}
static char* get_omitted_argument(const char* prompt, ...)
{
    char*   result;
    va_list args;

    va_start(args, prompt);
    result = get_missing_argument(prompt, args);
    va_end(args);

    return result;
}

static char* get_required_argument(const char* prompt, ...)
{
    char* result;

    if (optarg == 0)
    {
	va_list args;

	va_start(args, prompt);
	result = get_missing_argument(prompt, args);
	va_end(args);
    }
    else
    {
	result = Xstrdup(optarg);
	optarg = 0;
    }

    return result;
}

static char* get_optional_argument(const char* default_value)
{
    char* result;

    if (optarg == 0)
    {
	result = Xstrdup(default_value);
    }
    else
    {
	result = Xstrdup(optarg);
	optarg = 0;
    }

    return result;
}

static int get_next_option()
{
    int next_opt;

    opts_index = -1;
    next_opt = getopt_long(opts_argc, opts_argv, short_opts, long_opts, &opts_index);

    if (next_opt == ':')
    {
	// Missing parameter value
	//fprintf(stderr, "Missing value! opts_index=%d, optarg=%p, optopt=%d\n", opts_index, optarg, optopt);
	next_opt = optopt;
	optarg = 0;
    }

    return next_opt;
}

static int parse_listen_subopts(struct listen_config* listen)
{
    int c = EOF;

    do
    {
	while (c != EOF)
	{
	    int opt = c;
	    c = EOF;
	    switch (opt)
	    {
		case lopt_port:
		    if (listen->port)
		    {
			fprintf (stderr, "Duplicate --port for --listen!\n");
			exit(1);
		    }
		    else
		    {
			listen->port = get_required_argument("TCP port number for the listen");
		    }
		    break;
		default:
		    return opt;
	    }
	}
	c = get_next_option();
    } while (c != EOF);

    return c;
}

static int parse_cli_listen(struct cli_config* cli_cfg)
{
    int next_symbol;
    struct listen_config* listen = Xcalloc(1, sizeof(struct listen_config));

    if (cli_cfg->spawnd.listen_last == 0)
    {
	cli_cfg->spawnd.listen_list = listen;
    }
    else
    {
	cli_cfg->spawnd.listen_last->next_listen = listen;
    }
    cli_cfg->spawnd.listen_last = listen;
    cli_cfg->used = 1;

    next_symbol = parse_listen_subopts(listen);

    // Check this listen is complete
    if (listen->port == 0)
    {
	listen->port = get_omitted_argument("TCP port number for the listen");
    }

    return next_symbol;
}

static int parse_host_subopts(struct host_config* host)
{
    int c = EOF;

    do
    {
	while (c != EOF)
	{
	    int opt = c;
	    c = EOF;
	    switch (opt)
	    {
		case lopt_address:
		    if (host->address)
		    {
			fprintf (stderr, "Duplicate --address for --host %s!\n", host->host_name);
			exit(1);
		    }
		    else
		    {
			host->address = get_required_argument("address for host %s", host->host_name);
		    }
		    break;
		case lopt_key:
		    if (host->secret_key)
		    {
			fprintf (stderr, "Duplicate --key for --host %s!\n", host->host_name);
			exit(1);
		    }
		    else
		    {
			host->secret_key = get_required_argument("key for host %s", host->host_name);
		    }
		    break;
		default:
		    return opt;
	    }
	}
	c = get_next_option();
    } while (c != EOF);

    return c;
}

static int parse_cli_host(struct cli_config* cli_cfg)
{
    int next_symbol;
    struct host_config* host = Xcalloc(1, sizeof(struct host_config));

    if (cli_cfg->tacplus.host_last == 0)
    {
	cli_cfg->tacplus.host_list = host;
    }
    else
    {
	cli_cfg->tacplus.host_last->next_host = host;
    }
    cli_cfg->tacplus.host_last = host;
    cli_cfg->used = 1;

    host->host_name = get_optional_argument("any");
    next_symbol = parse_host_subopts(host);

    // Check this host is complete
    if (host->address == 0)
    {
	host->address = "0.0.0.0/0";
    }
    if (host->secret_key == 0)
    {
	host->secret_key = get_omitted_argument("key for host %s", host->host_name);
    }

    return next_symbol;
}

static int parse_cmd_subopts(struct cmd_config* cmd)
{
    int c = EOF;

    do
    {
	while (c != EOF)
	{
	    const char* mode = "permit";
	    struct access_config* access;
	    int opt = c;
	    c = EOF;
	    switch (opt)
	    {
		case lopt_deny:
		    mode = "deny";
		    /* fall through */
		case lopt_permit:
		    access = Xcalloc(1, sizeof(struct access_config));
		    if (cmd->access_last == 0)
		    {
			cmd->access_list = access;
		    }
		    else
		    {
			cmd->access_last->next_access = access;
		    }
		    access->access_type = Xstrdup(mode);
		    access->access_regex = get_optional_argument(".*");
		    break;
		default:
		    return opt;
	    }
	}
	c = get_next_option();
    } while (c != EOF);

    return c;
}

static int parse_user_subopts(struct user_config* user)
{
    int c = EOF;

    do
    {
	while (c != EOF)
	{
	    int opt = c;
	    c = EOF;
	    switch (opt)
	    {
		case lopt_password:
		    if (user->password)
		    {
			fprintf (stderr, "Duplicate --password for --user %s!\n", user->username);
			exit(1);
		    }
		    else
		    {
			user->password = get_required_argument("password for user %s", user->username);
		    }
		    break;
		case lopt_cmd:
		    if (user->service_list == 0)
		    {
			struct service_config* shell = Xcalloc(1, sizeof(struct service_config));
		    	shell->svc_name = "shell";
			user->service_list = shell;
			user->service_last = shell;
		    }
		    struct service_config* shell = user->service_list;
		    struct cmd_config* cmd = Xcalloc(1, sizeof(struct cmd_config));
		    if (shell->cmd_last == 0)
		    {
			shell->cmd_list = cmd;
		    }
		    else
		    {
			shell->cmd_last->next_cmd = cmd;
		    }
		    shell->cmd_last = cmd;
		    cmd->cmd_name = get_required_argument("command name for --cmd");
		    c = parse_cmd_subopts(cmd);
		    break;
		default:
		    return opt;
	    }
	}
	c = get_next_option();
    } while (c != EOF);

    return c;
}

static int parse_cli_user(struct cli_config* cli_cfg)
{
    int next_symbol;
    struct user_config* user = Xcalloc(1, sizeof(struct user_config));

    if (cli_cfg->tacplus.user_last == 0)
    {
	cli_cfg->tacplus.user_list = user;
    }
    else
    {
	cli_cfg->tacplus.user_last->next_user = user;
    }
    cli_cfg->tacplus.user_last = user;
    cli_cfg->used = 1;

    user->username = get_required_argument("name for the user");
    next_symbol = parse_user_subopts(user);

    // Check this user is complete
    if (user->password == 0)
    {
	user->password = get_omitted_argument("password for user %s", user->username);
    }

    return next_symbol;
}

static char* generate_more(char* existing, char* extra)
{
    if (existing)
    {
	existing = realloc (existing, strlen(existing) + strlen(extra) + 1);
	if (existing)
	{
	    strcat (existing, extra);
	}
    }
    else
    {
	existing = Xstrdup(extra);
    }

    return existing;
}

static char* generate_listen_config(char* so_far, struct listen_config* listen)
{
    so_far = generate_more (so_far, "\tlisten = {\n");

    if (listen->port)
    {
	so_far = generate_more (so_far, "\t\tport = ");
	so_far = generate_more (so_far, listen->port);
	so_far = generate_more (so_far, "\n");
    }

    so_far = generate_more (so_far, "\t}\n");
    return so_far;
}

static char* generate_spawnd_config(char* so_far, struct spawnd_config* spawnd_cfg)
{
    struct listen_config* listen;

    so_far = generate_more (so_far, "id = spawnd {\n");

    for (listen = spawnd_cfg->listen_list; listen; listen = listen->next_listen)
    {
	so_far = generate_listen_config (so_far, listen);
    }

    // Static items that are candidates for parameterising...
    so_far = generate_more (so_far, "\tspawn = {\n");
    so_far = generate_more (so_far, "\t        instances min = 1\n");
    so_far = generate_more (so_far, "\t        instances max = 10\n");
    so_far = generate_more (so_far, "\t}\n");
    so_far = generate_more (so_far, "\tbackground = no\n");


    so_far = generate_more (so_far, "}\n");
    return so_far;
}

static char* generate_host_config(char* so_far, struct host_config* host)
{
    so_far = generate_more (so_far, "\thost = ");
    so_far = generate_more (so_far, host->host_name);
    so_far = generate_more (so_far, " {\n");

    if (host->address)
    {
	so_far = generate_more (so_far, "\t\taddress = ");
	so_far = generate_more (so_far, host->address);
	so_far = generate_more (so_far, "\n");
    }

    if (host->secret_key)
    {
	so_far = generate_more (so_far, "\t\tkey = ");
	so_far = generate_more (so_far, host->secret_key);
	so_far = generate_more (so_far, "\n");
    }

    // Static items that are candidates for parameterising...
    // so_far = generate_more (so_far, "\t\tprompt = \"Welcome\\n\"\n");
    // so_far = generate_more (so_far, "\t\tenable 15 = clear secret\n");

    so_far = generate_more (so_far, "\t}\n");
    return so_far;
}

static char* generate_access_config(char* so_far, struct access_config* access)
{
    so_far = generate_more (so_far, "\t\t\t\t");
    so_far = generate_more (so_far, access->access_type);
    so_far = generate_more (so_far, " ");
    so_far = generate_more (so_far, access->access_regex);
    so_far = generate_more (so_far, "\n");

    return so_far;
}

static char* generate_cmd_config(char* so_far, struct cmd_config* cmd)
{
    struct access_config* access;

    so_far = generate_more (so_far, "\t\t\tcmd = ");
    so_far = generate_more (so_far, cmd->cmd_name);
    so_far = generate_more (so_far, " {\n");

    for (access = cmd->access_list; access; access = access->next_access)
    {
	so_far = generate_access_config (so_far, access);
    }

    so_far = generate_more (so_far, "\t\t\t}\n");

    return so_far;
}

static char* generate_service_config(char* so_far, struct service_config* svc)
{
    struct cmd_config* cmd;

    so_far = generate_more (so_far, "\t\tservice = ");
    so_far = generate_more (so_far, svc->svc_name);
    so_far = generate_more (so_far, " {\n");

    for (cmd = svc->cmd_list; cmd; cmd = cmd->next_cmd)
    {
	so_far = generate_cmd_config (so_far, cmd);
    }

    so_far = generate_more (so_far, "\t\t}\n");

    return so_far;
}

static char* generate_user_config(char* so_far, struct user_config* user)
{
    struct service_config* svc;

    so_far = generate_more (so_far, "\tuser = ");
    so_far = generate_more (so_far, user->username);
    so_far = generate_more (so_far, " {\n");

    if (user->password)
    {
	so_far = generate_more (so_far, "\t\tpassword = clear ");
	so_far = generate_more (so_far, user->password);
	so_far = generate_more (so_far, "\n");
    }

    // Static items that are candidates for parameterising...
    // so_far = generate_more (so_far, "\t\tmember = guest\n");

    for (svc = user->service_list; svc; svc = svc->next_service)
    {
	so_far = generate_service_config (so_far, svc);
    }

    so_far = generate_more (so_far, "\t}\n");
    return so_far;
}

static char* generate_tacplus_config(char* so_far, struct tacplus_config* tacplus_cfg)
{
    struct host_config* host;
    struct user_config* user;

    so_far = generate_more (so_far, "\nid = tac_plus {\n");

    // Static items that are candidates for parameterising...
    so_far = generate_more (so_far, "\tdebug = PACKET AUTHEN AUTHOR\n");
    so_far = generate_more (so_far, "\taccess log = output/access_4950\n");
    so_far = generate_more (so_far, "\taccounting log = output/acct_4950\n");

    for (host = tacplus_cfg->host_list; host; host = host->next_host)
    {
	so_far = generate_host_config (so_far, host);
    }

    // Static items that are candidates for parameterising...
    // so_far = generate_more (so_far, "\tgroup = guest {\n");
    // so_far = generate_more (so_far, "\t\tdefault service = permit\n");
    // so_far = generate_more (so_far, "\t\tenable = deny\n");
    // so_far = generate_more (so_far, "\t\tservice = shell {\n");
    // so_far = generate_more (so_far, "\t\t\tdefault command = permit\n");
    // so_far = generate_more (so_far, "\t\t\tdefault attribute = permit\n");
    // so_far = generate_more (so_far, "\t\t\tset priv-lvl = 1\n");
    // so_far = generate_more (so_far, "\t\t}\n");
    // so_far = generate_more (so_far, "\t}\n");


    for (user = tacplus_cfg->user_list; user; user = user->next_user)
    {
	so_far = generate_user_config (so_far, user);
    }

    so_far = generate_more (so_far, "}\n");
    return so_far;
}

static char* generate_cli_config(struct cli_config* cli_cfg)
{
    char* generated = 0;

    generated = generate_spawnd_config(generated, &cli_cfg->spawnd);
    generated = generate_tacplus_config(generated, &cli_cfg->tacplus);

    return generated;
}

int spawnd_main(int argc, char **argv, char **envp, char *id)
{
    pid_t pid;
    int c, devnull, i;
    struct spawnd_context *ctx = NULL;
    int socktype = 0;
    socklen_t socktypelen = sizeof(socktype);
    struct cli_config cli_conf;

    opts_argc = argc;
    opts_argv = argv;
    opts_index = 0;

    init_common_data();
    common_data.argv = dup_array(argv);
    common_data.envp = dup_array(envp);
    common_data.users_min = 20;
    common_data.users_max = 60;
    common_data.servers_min = 2;
    common_data.servers_max = 8;
    common_data.progname = Xstrdup(basename(argv[0]));
    common_data.version = VERSION "/Hydrix_0.2"
#ifdef WITH_PCRE
	"/PCRE"
#endif
#ifdef WITH_SSL
	"/DES"
#endif
#ifdef WITH_LWRES
	"/LWRES"
#endif
#ifdef WITH_CURL
	"/CURL"
#endif
	;
    logopen();

    memset(&cli_conf, 0, sizeof(cli_conf));

    memset(&spawnd_data, 0, sizeof(spawnd_data));
    get_exec_path(&spawnd_data.child_path, argv[0]);
    spawnd_data.overload = S_queue;
    common_data.progpath = Xstrdup(spawnd_data.child_path);
    spawnd_data.keepintvl = -1;
    spawnd_data.keepcnt = -1;
    spawnd_data.keepidle = -1;

    if (!getsockopt(0, SOL_SOCKET, SO_TYPE, &socktype, &socktypelen))
	switch (socktype) {
	case SOCK_DGRAM:
	    logmsg("FATAL: Recursive execution prohibited.");
	    scm_fatal();
	case SOCK_STREAM:
	    spawnd_data.inetd = 1;
	}

    //while ((c = getopt(argc, argv, "vPd:i:p:c:bf1")) != EOF)
    c = EOF;
    do
    {
	while (c != EOF)
	{
	    int opt = c;
	    c = EOF;
	    switch (opt)
	    {
		case 'c':
		    common_data.alt_config = Xstrdup(optarg);
		    break;
		case 'v':
		    common_data.version_only = 1;
		    break;
		case 'P':
		    common_data.parse_only = 1;
		    break;
		case 'd':
		    common_data.debug = atoi(optarg) & DEBUG_ALL_FLAG;
		    break;
		case 'b':
		    spawnd_data.background = 1;
		    spawnd_data.background_lock = 1;
		    break;
		case 'f':
		    spawnd_data.background = 0;
		    spawnd_data.background_lock = 1;
		    break;
		case 'i':
		    strset(&spawnd_data.child_id, optarg);
		    break;
		case 'p':
		    strset(&spawnd_data.pidfile, optarg);
		    spawnd_data.pidfile_lock = 1;
		    break;
		case '1':
		    common_data.singleprocess = 1;
		    break;
		case lopt_print:
		    cli_conf.print = 1;
		    break;
		case lopt_listen:
		    c = parse_cli_listen (&cli_conf);
		    break;
		case lopt_host:
		    c = parse_cli_host (&cli_conf);
		    break;
		case lopt_user:
		    c = parse_cli_user (&cli_conf);
		    break;
		default:
		    common_usage();
	    }
	}
	c = get_next_option();
    } while (c != EOF);

    if (argc == optind && common_data.version_only) {
	int status;
	spawnd_spawn_child(NULL);
	waitpid(-1, &status, 0);
	exit(WEXITSTATUS(status));
    }

    if (cli_conf.used)
    {
	common_data.alt_config = generate_cli_config(&cli_conf);
    }

    if (common_data.alt_config)
    {
	if (cli_conf.print)
	{
	    printf("generated configuration is:\n%s\n======\n", common_data.alt_config);
	}
	if (argc != optind && argc != optind+1)
	{
	    printf("argc=%d optind=%d\n", argc, optind);
	    common_usage();
	}
	if (common_data.ipc_key == 0)
	{
	    common_data.ipc_key = 456;
	}
	if (id == 0)
	{
	     id = common_data.progname;
	}
	if (ipc_create (common_data.alt_config, strlen(common_data.alt_config) + 1) != 0)
	{
	    fprintf (stderr, "ipc_create() failed!\n");
	    exit(1);
	}
	else
	{
	    common_data.conffile = common_data.ipc_url;
	    cfg_read_config(common_data.ipc_url, spawnd_parse_decls, id);
	}    
    }
    else
    {
	if (argc != optind + 1 && argc != optind + 2)
	{
	    common_usage();
	}
	common_data.conffile = Xstrdup(argv[optind]);
	cfg_read_config(argv[optind], spawnd_parse_decls, id ? id : (argv[optind + 1] ? argv[optind + 1] : common_data.progname));
    }

    strset(&spawnd_data.child_config, argv[optind]);

    if (argv[optind + 1])
	common_data.id = Xstrdup(argv[optind + 1]);

    logmsg("startup%s (version " VERSION ")", spawnd_data.inetd ? " via inetd" : "");

    umask(077);

    if (spawnd_data.inetd) {
	int one = 1;
	socklen_t sulen = sizeof(sockaddr_union);
	ctx = spawnd_new_context(NULL);
	ctx->fn = 0;
	ctx->listen_backlog = 128;
	setsockopt(0, SOL_SOCKET, SO_REUSEADDR, (char *) &one, (socklen_t) sizeof(one));
	fcntl(ctx->fn, F_SETFD, fcntl(ctx->fn, F_GETFD, 0) | FD_CLOEXEC);
	fcntl(ctx->fn, F_SETFL, O_NONBLOCK);
	if (0 > getsockname(0, &ctx->sa.sa, &sulen)) {
	    logerr("getsockname (%s:%d)", __FILE__, __LINE__);
	} else {
	    spawnd_data.listener_arr = Xrealloc(spawnd_data.listener_arr, (spawnd_data.listeners_max + 1) * sizeof(struct spawnd_context *));
	    spawnd_data.listener_arr[spawnd_data.listeners_max++] = ctx;
	}
	spawnd_data.background = 0;
	spawnd_data.background_lock = 1;
    }

    spawnd_data.retry_delay = 0;

#if 0
    common_data.conffile = strdup(argv[optind]);
    if (argv[optind + 1])
	common_data.id = strdup(argv[optind + 1]);
#endif


    common_data.users_max_total = common_data.users_max * common_data.servers_max;

    if (common_data.servers_max < common_data.servers_min)
	common_data.servers_min = common_data.servers_max;

    switch (spawnd_data.overload) {
    case S_reset:
	spawnd_data.overload_hint = "resetting";
	break;
    case S_close:
	spawnd_data.overload_hint = "immediately closing";
	break;
    default:
	spawnd_data.overload_hint = "queueing";
    }

    if (common_data.parse_only || common_data.version_only) {
	int status;
	spawnd_spawn_child(NULL);
	waitpid(-1, &status, 0);
	exit(WEXITSTATUS(status));
    }

    setproctitle_init(argv, envp);

    umask(022);

    if (!spawnd_data.listeners_max) {
	logmsg("FATAL: No listeners defined.");
	exit(EX_OSERR);
    }

    if (spawnd_data.background)
	switch (pid = fork()) {
	case 0:
	    devnull = open("/dev/null", O_RDWR);
	    dup2(devnull, 0);
	    dup2(devnull, 1);
	    if (!common_data.debug_redirected)
		dup2(devnull, 2);
	    close(devnull);
	    setsid();
	    break;
	case -1:
	    logerr("fork (%s:%d)", __FILE__, __LINE__);
	    exit(EX_OSERR);
	default:
	    // logmsg("fork() succeeded. New PID is %u.", (u_int) pid);
	    exit(EX_OK);
	}
#ifdef DEBUG
    debug_setpid();
#endif				/* DEBUG */

    if (spawnd_data.pidfile && !(common_data.pidfile = pid_write(spawnd_data.pidfile)))
	logerr("pid_write(%s) (%s:%d)", spawnd_data.pidfile, __FILE__, __LINE__);

    common_data.io = io_init();

    for (i = 0; i < spawnd_data.listeners_max; i++) {
	if (spawnd_data.listener_arr[i]->keepcnt < 0)
	    spawnd_data.listener_arr[i]->keepcnt = spawnd_data.keepcnt;
	if (spawnd_data.listener_arr[i]->keepidle < 0)
	    spawnd_data.listener_arr[i]->keepidle = spawnd_data.keepidle;
	if (spawnd_data.listener_arr[i]->keepintvl < 0)
	    spawnd_data.listener_arr[i]->keepintvl = spawnd_data.keepintvl;

	spawnd_data.listener_arr[i]->io = common_data.io;
	spawnd_bind_listener(spawnd_data.listener_arr[i], spawnd_data.listener_arr[i]->fn);
    }

#ifdef BROKEN_FD_PASSING
    common_data.singleprocess = 1;
#endif

    if (common_data.singleprocess) {
	logmsg("Warning: Running in degraded mode. This is unsuitable for real-life usage.");
	common_data.servers_min = 1;
	common_data.servers_max = 1;
	common_data.scm_send_msg = fakescm_send_msg;
	common_data.scm_recv_msg = fakescm_recv_msg;
	return 0;
    }

    ctx = spawnd_new_context(common_data.io);
    io_sched_add(common_data.io, ctx, (void *) periodics, (time_t) 10, (suseconds_t) 0);

    spawnd_data.server_arr = Xcalloc(common_data.servers_max, sizeof(struct spawnd_context *));

    spawnd_data.tracking_size = 1024;

    spawnd_setup_signals();
    setup_sig_segv(common_data.coredumpdir, common_data.gcorepath, common_data.debug_cmd);

    while (common_data.servers_cur < common_data.servers_min)
	spawnd_add_child();

    set_proctitle(ACCEPT);

    io_main(common_data.io);
}

void scm_main(int argc, char **argv, char **envp)
{
    extern char *optarg;
    extern int optind;
    int socktype = 0, c;
    socklen_t socktypelen = (socklen_t) sizeof(socktype);

    init_common_data();
    common_data.progname = strdup(basename(argv[0]));

    common_data.version = VERSION
#ifdef WITH_PCRE
	"/PCRE"
#endif
#ifdef WITH_SSL
	"/DES"
#endif
#ifdef WITH_LWRES
	"/LWRES"
#endif
#ifdef WITH_CURL
	"/CURL"
#endif
	;
    logopen();

    if (getsockopt(0, SOL_SOCKET, SO_TYPE, &socktype, &socktypelen)
	|| socktype != SOCK_DGRAM) {
	spawnd_main(argc, argv, envp, "spawnd");
	if (common_data.singleprocess)
	    return;
	exit(0);
    }

    while ((c = getopt(argc, argv, "vPd:c:")) != EOF)
	switch (c) {
	case 'v':
	    fprintf(stderr, "%s version %s\n", common_data.progname, common_data.version);
	    exit(EX_OK);
	case 'P':
	    common_data.parse_only = 1;
	    break;
	case 'd':
	    common_data.debug = atoi(optarg) & DEBUG_ALL_FLAG;
	    break;
	default:
	    common_usage();
	}
	if (argc != optind && argc != optind + 1 && argc != optind + 2)
	        common_usage();
}