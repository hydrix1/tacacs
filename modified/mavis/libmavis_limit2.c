/*
 * libmavis_limit.c
 *
 * (C)1998-2011 by Marc Huber <Marc.Huber@web.de>
 * All rights reserved.
 *
 * $Id: libmavis_limit2.c,v 1.9 2015/03/14 06:11:28 marc Exp marc $
 *
 */

#define __MAVIS_limit
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <dlfcn.h>

#include "misc/sysconf.h"

#include "log.h"
#include "debug.h"
#include "misc/memops.h"
#include "misc/net.h"
#include "misc/rb.h"

static const char rcsid[] __attribute__ ((used)) = "$Id: libmavis_limit2.c,v 1.9 2015/03/14 06:11:28 marc Exp marc $";

#define MAVIS_CTX_PRIVATE		\
	time_t lastpurge;		\
	u_int addr_blacklist_count;	\
	time_t addr_blacklist_time;	\
	time_t addr_maxage;		\
	u_int user_blacklist_count;	\
	time_t user_blacklist_time;	\
	time_t user_maxage;		\
	time_t purge_outdated;		\
	rb_tree_t *addrs;

#include "mavis.h"

struct addr_entry {
    time_t expire;
    u_int count;
    struct in6_addr addr;
    rb_tree_t *users;
};

struct user_entry {
    time_t expire;
    u_int count;
    char user[1];
};

static int compare_addr(const void *a, const void *b)
{
    return v6_cmp(&((struct addr_entry *) a)->addr, &((struct addr_entry *) b)->addr);
}

static int compare_user(const void *a, const void *b)
{
    return strcmp(((struct user_entry *) a)->user, ((struct user_entry *) b)->user);
}

static void free_payload(void *payload)
{
    free(payload);
}

static void add_addr(mavis_ctx * mcx, char *user, char *addr)
{
    if (user && addr) {
	size_t userlen = strlen(user);
	struct addr_entry *addr_entry = alloca(sizeof(addr_entry));
	struct user_entry *user_entry = alloca(sizeof(user_entry) + userlen);
	rb_node_t *t;

	if (v6_ptoh(&addr_entry->addr, NULL, addr))
	    return;

	t = RB_search(mcx->addrs, addr_entry);
	if (t)
	    addr_entry = RB_payload(t, struct addr_entry *);
	if (!addr_entry) {
	    addr_entry = Xcalloc(1, sizeof(struct addr_entry));
	    addr_entry->count = 1;
	    v6_ptoh(&addr_entry->addr, NULL, addr);
	    addr_entry->expire = io_now.tv_sec + mcx->addr_maxage;
	    RB_insert(mcx->addrs, addr_entry);
	}
	if (!addr_entry->users)
	    addr_entry->users = RB_tree_new(compare_user, free_payload);

	strcpy(user_entry->user, user);
	t = RB_search(addr_entry->users, user_entry);
	if (t)
	    user_entry = RB_payload(t, struct user_entry *);
	if (!user_entry) {
	    user_entry = Xcalloc(1, sizeof(struct user_entry) + userlen);
	    strcpy(user_entry->user, user);
	    user_entry->expire = io_now.tv_sec + mcx->user_maxage;
	    RB_insert(addr_entry->users, user_entry);
	}

	if (io_now.tv_sec > addr_entry->expire) {
	    addr_entry->count = 0;
	    addr_entry->expire = io_now.tv_sec + mcx->addr_maxage;
	    addr_entry->count++;
	}

	if (io_now.tv_sec > user_entry->expire) {
	    user_entry->count = 0;
	    user_entry->expire = io_now.tv_sec + mcx->user_maxage;
	    user_entry->count++;
	}
    }
#if 0
    add_add_remote(mcx, user, addr);
#endif
}

static struct addr_entry *find_addr(rb_tree_t * addrs, char *addr)
{
    struct addr_entry addr_entry;
    rb_node_t *t = NULL;

    if (!v6_ptoh(&addr_entry.addr, NULL, addr))
	t = RB_search(addrs, &addr_entry);

    return t ? RB_payload(t, struct addr_entry *) : NULL;
}

static struct user_entry *find_user(rb_tree_t * users, char *user)
{
    size_t userlen = strlen(user);
    struct user_entry *user_entry = alloca(sizeof(user_entry) + userlen);
    rb_node_t *t;

    strcpy(user_entry->user, user);
    t = RB_search(users, &user_entry);

    return t ? RB_payload(t, struct user_entry *) : NULL;
}

static void gc_users(rb_tree_t * users)
{
    rb_node_t *t, *u;

    if (users)
	for (t = RB_first(users); t; t = u)
	    if (u = RB_next(t), RB_payload(t, struct user_entry *)->expire < io_now.tv_sec)
		 RB_delete(users, t);
}

static void gc_addrs(rb_tree_t * addr)
{
    rb_node_t *t, *u;

    if (addr)
	for (t = RB_first(addr); t; t = u)
	    if (u = RB_next(t), RB_payload(t, struct addr_entry *)->expire < io_now.tv_sec) {
		rb_tree_t *users = RB_payload(u, struct addr_entry *)->users;
		gc_users(users);
		if (RB_empty(users))
		    RB_tree_delete(users);
		RB_delete(addr, t);
	    }
}

static void garbage_collection(mavis_ctx * mcx)
{
    DebugIn(DEBUG_PROC);

    gc_addrs(mcx->addrs);

    DebugOut(DEBUG_PROC);
}

#define HAVE_mavis_drop_in
static void mavis_drop_in(mavis_ctx * mcx)
{
    RB_tree_delete(mcx->addrs);
}

/*
purge period =...blacklist time =...blacklist count =...
*/
#define HAVE_mavis_parse_in
static int mavis_parse_in(mavis_ctx * mcx, struct sym *sym)
{
    while (1) {
	switch (sym->code) {
	case S_script:
	    mavis_script_parse(mcx, sym);
	    continue;
	case S_purge:
	    sym_get(sym);
	    parse(sym, S_period);
	    parse(sym, S_equal);
	    mcx->purge_outdated = (time_t) parse_int(sym);
	    continue;
	case S_blacklist:
	    sym_get(sym);
	    switch (sym->code) {
	    case S_time:
		sym_get(sym);
		parse(sym, S_equal);
		mcx->addr_blacklist_time = (time_t) parse_int(sym);;
		mcx->user_blacklist_time = (time_t) parse_int(sym);;
		break;
	    case S_count:
		sym_get(sym);
		parse(sym, S_equal);
		mcx->addr_blacklist_count = (u_int) parse_int(sym);;
		mcx->user_blacklist_count = (u_int) parse_int(sym);;
		break;
	    default:
		parse_error_expect(sym, S_time, S_count, S_unknown);
	    }
	    continue;
	case S_user:
	    sym_get(sym);
	    parse(sym, S_blacklist);
	    switch (sym->code) {
	    case S_time:
		sym_get(sym);
		parse(sym, S_equal);
		mcx->user_blacklist_time = (time_t) parse_int(sym);;
		break;
	    case S_count:
		sym_get(sym);
		parse(sym, S_equal);
		mcx->user_blacklist_count = (u_int) parse_int(sym);;
		break;
	    default:
		parse_error_expect(sym, S_blacklist);
	    }
	    continue;
	case S_address:
	    sym_get(sym);
	    parse(sym, S_blacklist);
	    switch (sym->code) {
	    case S_time:
		sym_get(sym);
		parse(sym, S_equal);
		mcx->addr_blacklist_time = (time_t) parse_int(sym);;
		break;
	    case S_count:
		sym_get(sym);
		parse(sym, S_equal);
		mcx->addr_blacklist_count = (u_int) parse_int(sym);;
		break;
	    default:
		parse_error_expect(sym, S_blacklist, S_unknown);
	    }
	    continue;
	case S_eof:
	case S_closebra:
	    return MAVIS_CONF_OK;
	default:
	    parse_error_expect(sym, S_script, S_purge, S_blacklist, S_user, S_address, S_expire, S_closebra, S_unknown);
	}
    }
}

#define HAVE_mavis_init_in
static int mavis_init_in(mavis_ctx * mcx)
{
    if (!mcx->addrs) {
	mcx->addrs = RB_tree_new(compare_addr, free_payload);
	mcx->lastpurge = io_now.tv_sec;
    }
    return MAVIS_INIT_OK;
}

#define HAVE_mavis_send_in
static int mavis_send_in(mavis_ctx * mcx, av_ctx ** ac)
{
    struct addr_entry *addr_entry = NULL;
    struct user_entry *user_entry = NULL;
    char *t, *addr, *user;
    t = av_get(*ac, AV_A_TYPE);
    if (!t)
	return MAVIS_FINAL;
    if (io_now.tv_sec > mcx->lastpurge + mcx->purge_outdated) {
	garbage_collection(mcx);
	mcx->lastpurge = io_now.tv_sec;
    }

    addr = av_get(*ac, AV_A_IPADDR);
    if (mcx->addr_blacklist_count && addr) {
	addr_entry = find_addr(mcx->addrs, addr);
	if (addr_entry && addr_entry->count >= mcx->addr_blacklist_count && addr_entry->expire > io_now.tv_sec) {
	    av_set(*ac, AV_A_RESULT, AV_V_RESULT_FAIL);
	    av_setf(*ac, AV_A_COMMENT, "client blacklisted for %ld seconds", (long) addr_entry->expire - io_now.tv_sec);
	    return MAVIS_FINAL;
	}
	user = av_get(*ac, AV_A_USER);
	if (mcx->user_blacklist_count && addr_entry && user) {
	    user_entry = find_user(addr_entry->users, user);
	    if (user_entry && user_entry->count >= mcx->user_blacklist_count && user_entry->expire > io_now.tv_sec) {
		av_set(*ac, AV_A_RESULT, AV_V_RESULT_FAIL);
		av_setf(*ac, AV_A_COMMENT, "user blacklisted for %ld seconds", (long) addr_entry->expire - io_now.tv_sec);
		return MAVIS_FINAL;
	    }
	}
    }
# if 0
    if (!addr_entry && !user_entry) {
    }
#endif

    return MAVIS_DOWN;
}

#define HAVE_mavis_recv_out
static int mavis_recv_out(mavis_ctx * mcx, av_ctx ** ac)
{
    char *t = av_get(*ac, AV_A_TYPE);
    char *u = av_get(*ac, AV_A_USER);
    char *i = av_get(*ac, AV_A_IPADDR);
    char *r = av_get(*ac, AV_A_RESULT);
    if (!r)
	r = AV_V_RESULT_FAIL;
    if (t && i && u) {
	if (!strcmp(r, AV_V_RESULT_FAIL)
	    && (!strcmp(t, AV_V_TYPE_WWW)
		|| !strcmp(t, AV_V_TYPE_FTP)
		|| !strcmp(t, AV_V_TYPE_TACPLUS)))
	    add_addr(mcx, u, i);
    }

    return MAVIS_FINAL;
}

#define HAVE_mavis_new
static void mavis_new(mavis_ctx * mcx)
{
    mcx->addr_blacklist_time = 300;
    mcx->user_blacklist_time = 300;
    mcx->purge_outdated = 300;
}

#define MAVIS_name "limit"
#include "mavis_glue.c"


#ifdef STATIC_MODULES

struct module_defn mod_limit2 = 
{
    .name = "limit2",
    .new =  Mavis_new,
    .append = Mavis_append, 
    .init = Mavis_init,
    .parse = Mavis_parse,
    .send = Mavis_send,
    .recv = Mavis_recv,
    .cancel = Mavis_cancel,
    .drop = Mavis_drop
};

#endif

