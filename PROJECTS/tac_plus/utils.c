/*
   Copyright (C) 1999-2011 Marc Huber (Marc.Huber@web.de)
   All rights reserved.

   Redistribution and use in source and binary  forms,  with or without
   modification, are permitted provided  that  the following conditions
   are met:

   1. Redistributions of source code  must  retain  the above copyright
      notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions  and  the following disclaimer in
      the  documentation  and/or  other  materials  provided  with  the
      distribution.

   3. The end-user documentation  included with the redistribution,  if
      any, must include the following acknowledgment:

          This product includes software developed by Marc Huber
	  (Marc.Huber@web.de).

      Alternately,  this  acknowledgment  may  appear  in  the software
      itself, if and wherever such third-party acknowledgments normally
      appear.

   THIS SOFTWARE IS  PROVIDED  ``AS IS''  AND  ANY EXPRESSED OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL  ITS  AUTHOR  BE  LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED  TO,  PROCUREMENT OF  SUBSTITUTE  GOODS OR SERVICES;
   LOSS OF USE,  DATA,  OR PROFITS;  OR  BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY,  WHETHER IN CONTRACT,  STRICT
   LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN
   ANY WAY OUT OF THE  USE  OF  THIS  SOFTWARE,  EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
 */

#include "headers.h"
#include "misc/buffer.h"

#ifdef WITH_PCRE
# include <pcre.h>
#endif

# include <regex.h>

static const char rcsid[] __attribute__ ((used)) = "$Id: utils.c,v 1.96 2015/03/14 06:10:28 marc Exp $";

void *mempool_malloc(rb_tree_t * pool, size_t size)
{
    void *p = calloc(1, size ? size : 1);

    if (p) {
	if (pool)
	    RB_insert(pool, p);
	return p;
    }
    report(NULL, LOG_ERR, ~0, "malloc %d failure", (int) size);
    tac_exit(EX_OSERR);
}

void *mempool_realloc(rb_tree_t * pool, void *p, size_t size)
{
    if (p) {
	if (pool) {
	    rb_node_t *rbn = RB_search(pool, p);
	    if (rbn) {
		RB_payload_unlink(rbn);
		RB_delete(pool, rbn);
	    }
	}
	p = realloc(p, size);
	if (p) {
	    if (pool)
		RB_insert(pool, p);
	    return p;
	}
    } else
	return mempool_malloc(pool, size);

    report(NULL, LOG_ERR, ~0, "realloc %d failure", (int) size);
    tac_exit(EX_OSERR);
}

void mempool_free(rb_tree_t * pool, void *ptr)
{
    void **m = ptr;

    if (*m) {
	if (pool) {
	    rb_node_t *rbn = RB_search(pool, *m);
	    if (rbn) {
		RB_delete(pool, rbn);
		*m = NULL;
	    } else
		report(NULL, LOG_DEBUG, ~0, "potential double-free attempt on %p", *m);
	} else
	    free(*m);
    }
}

static int pool_cmp(const void *a, const void *b)
{
    return (a < b) ? -1 : ((a == b) ? 0 : +1);
}

void mempool_destroy(rb_tree_t * pool)
{
    RB_tree_delete(pool);
}

rb_tree_t *mempool_create(void)
{
    return RB_tree_new(pool_cmp, free);
}

#ifdef WITH_PCRE
rb_tree_t *tac_pcrepool_create(void)
{
    return RB_tree_new(pool_cmp, (void (*)(void *)) pcre_free);
}
#endif

rb_tree_t *tac_regpool_create(void)
{
    return RB_tree_new(pool_cmp, (void (*)(void *)) regfree);
}

char *mempool_strdup(rb_tree_t * pool, char *p)
{
    char *n = strdup(p);

    if (n) {
	if (pool)
	    RB_insert(pool, n);
	return n;
    }
    report(NULL, LOG_ERR, ~0, "strdup allocation failure");
    tac_exit(EX_OSERR);
}

char *mempool_strndup(rb_tree_t * pool, u_char * p, int len)
{
    char *string;
    int new_len = len;

    /* 
     * Add space for a null terminator if needed. Also, no telling
     * what various mallocs will do when asked for a length of zero.
     */
    if (!len || p[len - 1])
	new_len++;

    string = mempool_malloc(pool, new_len);

    memcpy(string, p, len);
    return string;
}

int tac_exit(int status)
{
    report(NULL, LOG_DEBUG, ~0, "exit status=%d", status);
    exit(status);
}

static void create_dirs(char *path)
{
    char *p = path;
    while ((p = strchr(p + 1, '/'))) {
	*p = 0;
	mkdir(path, config.mask | ((0111) & (config.mask >> 2)));
	*p = '/';
    }
}

static int tac_lock(int lockfd, int locktype)
{
    struct flock flock;

    memset(&flock, 0, sizeof(flock));
    flock.l_type = locktype;
    flock.l_whence = SEEK_SET;
    return fcntl(lockfd, F_SETLK, &flock);
}

#define tac_lockfd(A) tac_lock(A,F_WRLCK)
#define tac_unlockfd(A) tac_lock(A,F_UNLCK)

struct logfile {
    char *format;		/* log file format specification */
    char *name;			/* log file specification */
    struct context_logfile *ctx;	/* current log context */
    void (*log_write) (struct logfile *, char *, size_t);
    void (*log_flush) (struct logfile *);
     BISTATE(flag_syslog);
     BISTATE(flag_sync);
     BISTATE(flag_pipe);
     BISTATE(flag_staticpath);
    int syslog_priority;
    char *date_format;
    char *log_separator;
    size_t log_separator_len;
    time_t last;
};

static void log_start_one(struct logfile *, struct context_logfile *);

static void logdied(pid_t pid __attribute__ ((unused)), struct context_logfile *ctx, int status __attribute__ ((unused)))
{
    if (ctx) {
	io_close(common_data.io, ctx->fd);
	ctx->lf->ctx = NULL;
	if (ctx->buf) {
	    log_start_one(ctx->lf, ctx);
	    io_set_o(common_data.io, ctx->fd);
	}
    }
}

static void logdied_handler(struct context_logfile *ctx, int cur __attribute__ ((unused)))
{
    io_child_ign(ctx->pid);
    logdied(ctx->pid, ctx, 0);
}

static void logwrite_retry(struct context_logfile *ctx, int cur __attribute__ ((unused)))
{
    io_sched_del(common_data.io, ctx, (void *) logwrite_retry);
    io_set_o(common_data.io, ctx->fd);
}

static void logwrite(struct context_logfile *ctx, int cur)
{
    struct buffer *b = ctx->buf;
    if (b) {
	if (!ctx->lf->flag_pipe && tac_lockfd(cur)) {
	    io_clr_o(common_data.io, cur);
	    io_sched_add(common_data.io, ctx, (void *) logwrite_retry, 1, 0);
	    return;
	}

	if (!ctx->lf->flag_pipe)
	    lseek(cur, 0, SEEK_END);

	while (b) {
	    ssize_t len = write(cur, b->buf + b->offset,
				b->length - b->offset);
	    if (len < 0 && errno == EAGAIN) {
		if (!ctx->lf->flag_pipe)
		    tac_unlockfd(cur);
		io_clr_o(common_data.io, cur);
		io_sched_add(common_data.io, ctx, (void *) logwrite_retry, 1, 0);
		return;
	    }
	    if (len < 0) {
		logdied_handler(ctx, cur);
	    } else {
		off_t o = (off_t) len;
		ctx->buf = buffer_release(ctx->buf, &o);
		if (!ctx->buf && ctx->dying) {
		    if (!ctx->lf->flag_pipe)
			tac_unlockfd(cur);
		    io_clr_o(common_data.io, cur);
		    io_close(common_data.io, cur);
		    ctx->lf->ctx = NULL;
		    free(ctx);
		    return;
		}
	    }
	    b = ctx->buf;
	}

	if (!ctx->lf->flag_pipe)
	    tac_unlockfd(cur);
    }
    io_clr_o(common_data.io, cur);
}

static void logwrite_sync(struct context_logfile *ctx, int cur)
{
    while (ctx->buf) {
	struct iovec v[10];
	int count = 10;
	buffer_setv(ctx->buf, v, &count, buffer_getlen(ctx->buf));
	if (count) {
	    ssize_t l = writev(cur, v, count);
	    if (l < 0) {
		//FIXME. Disk full, probably.
		return;
	    }
	    ctx->buf = buffer_release(ctx->buf, (off_t *) & l);
	}
    }
}

static void log_start_one(struct logfile *lf, struct context_logfile *deadctx)
{
    char newpath[PATH_MAX + 1];
    char *path = NULL;
    int cur = -1;

    if (deadctx) {
	path = deadctx->path;
    } else if (!lf->flag_syslog) {
	if (lf->flag_staticpath) {
	    path = lf->format;
	} else {
	    time_t dummy = (time_t) io_now.tv_sec;
	    struct tm *tm = localtime(&dummy);
	    if (!strftime(newpath, sizeof(newpath), lf->format, tm)) {
		report(NULL, LOG_DEBUG, -1, "strftime failed for %s", lf->format);
		return;
	    }
	    path = newpath;
	    if (lf->ctx && strcmp(path, lf->ctx->path)) {
		if (lf->flag_sync) {
		    while (lf->ctx->buf) {
			struct iovec v[10];
			int count = 10;
			size_t len = buffer_getlen(lf->ctx->buf);
			buffer_setv(lf->ctx->buf, v, &count, len);
			if (count) {
			    count = writev(lf->ctx->fd, v, count);
			    lf->ctx->buf = buffer_release(lf->ctx->buf, (off_t *) & len);
			}
		    }
		    close(lf->ctx->fd);
		    free(lf->ctx);
		    lf->ctx = NULL;
		} else {
		    if (lf->ctx->buf == NULL) {
			if (lf->ctx->fd > -1)
			    io_close(common_data.io, lf->ctx->fd);
			free(lf->ctx);
			lf->ctx = NULL;
		    } else {
			lf->ctx->dying = 1;
			lf->ctx = NULL;
		    }
		}
	    }
	}
    }

    if (!lf->ctx) {

	if (lf->last + 5 > io_now.tv_sec) {
	    report(NULL, LOG_INFO, -1, "\"%s\" respawning too fast", lf->format);
	    return;
	}

	lf->last = io_now.tv_sec;

	if (lf->flag_pipe) {
	    int fds[2], flags;
	    pid_t pid;

	    if (pipe(fds)) {
		report(NULL, LOG_DEBUG, -1, "pipe (%s:%d): %s", __FILE__, __LINE__, strerror(errno));
		return;
	    }
	    switch ((pid = io_child_fork((void (*)(pid_t, void *, int)) logdied, deadctx))) {
	    case 0:
		io_destroy(common_data.io, NULL);
		close(fds[1]);
		if (fds[0]) {
		    dup2(fds[0], 0);
		    close(fds[0]);
		}

		/*
		 * Casting NULL to (char *) NULL to avoid GCC warnings
		 * observed on OpenBSD ...
		 */
		execl("/bin/sh", "sh", "-c", path, (char *) NULL);
		execl("/usr/bin/sh", "sh", "-c", path, (char *) NULL);

		report(NULL, LOG_DEBUG, -1, "execl (%s, ...) (%s:%d)", path, __FILE__, __LINE__);
		exit(EX_OSERR);
	    case -1:
		report(NULL, LOG_DEBUG, -1, "fork (%s:%d): %s", __FILE__, __LINE__, strerror(errno));
		break;
	    default:
		close(fds[0]);
		flags = fcntl(fds[1], F_GETFD, 0) | FD_CLOEXEC;
		fcntl(fds[1], F_SETFD, flags);
		cur = fds[1];
		if (deadctx)
		    lf->ctx = deadctx;
		else {
		    lf->ctx = calloc(1, sizeof(struct context_logfile));
		    strncpy(lf->ctx->path, path, PATH_MAX);
		    io_child_set(pid, (void (*)(pid_t, void *, int))
				 logdied, (void *) lf->ctx);
		}
		lf->ctx->pid = pid;
	    }
	} else if (lf->flag_syslog) {
	    lf->ctx = calloc(1, sizeof(struct context_logfile));
	    lf->flag_sync = 1;
	} else {
	    cur = open(path, O_CREAT | O_WRONLY | O_APPEND, config.mask);
	    if (cur < 0 && errno != EACCES) {
		create_dirs(path);
		cur = open(path, O_CREAT | O_WRONLY | O_APPEND, config.mask);
	    }
	    if (cur > -1 && !lf->ctx) {
		lf->ctx = calloc(1, sizeof(struct context_logfile));
		strncpy(lf->ctx->path, path, PATH_MAX);
	    }
	}

	if (lf->ctx) {
	    lf->ctx->fd = cur;
	    lf->ctx->lf = lf;

	    if (cur > -1 && !lf->flag_sync) {
		io_register(common_data.io, cur, lf->ctx);
		io_set_cb_h(common_data.io, cur, (void *) logdied_handler);
		io_set_cb_e(common_data.io, cur, (void *) logdied_handler);
		io_set_cb_o(common_data.io, cur, (void *) logwrite);

		fcntl(cur, F_SETFL, O_NONBLOCK);
	    }
	}
    }
}

void log_start(rb_tree_t * rbt)
{
    rb_node_t *rbn, *rbnext;
    for (rbn = RB_first(rbt); rbn; rbn = rbnext) {
	struct logfile *lf = RB_payload(rbn, struct logfile *);
	rbnext = RB_next(rbn);
	log_start_one(lf, NULL);
    }
}

void log_write_date(rb_tree_t * rbt)
{
    char dstr[1024];
    time_t dummy = (time_t) io_now.tv_sec;
    struct tm *tm = localtime(&dummy);
    rb_node_t *rbn, *rbnext;
    for (rbn = RB_first(rbt); rbn; rbn = rbnext) {
	struct logfile *lf = RB_payload(rbn, struct logfile *);
	if (lf->date_format[0]) {
	    strftime(dstr, sizeof(dstr), lf->date_format, tm);
	    lf->log_write(lf, dstr, strlen(dstr));
	    if (lf->log_separator_len)
		lf->log_write(lf, lf->log_separator, lf->log_separator_len);
	}
	rbnext = RB_next(rbn);
    }
}

void log_write_separator(rb_tree_t * rbt)
{
    rb_node_t *rbn, *rbnext;
    for (rbn = RB_first(rbt); rbn; rbn = rbnext) {
	struct logfile *lf = RB_payload(rbn, struct logfile *);
	rbnext = RB_next(rbn);
	if (lf->log_separator_len)
	    lf->log_write(lf, lf->log_separator, lf->log_separator_len);
	log_start_one(lf, NULL);
    }
}

static void log_write_async(struct logfile *lf, char *buf, size_t len)
{
    if (lf->ctx) {
	if (buffer_getlen(lf->ctx->buf) > 64000)	/* FIXME? */
	    lf->ctx->buf = buffer_free_all(lf->ctx->buf);
	lf->ctx->buf = buffer_write(lf->ctx->buf, buf, len);
	io_set_o(common_data.io, lf->ctx->fd);
    }
}

static void log_write_sync(struct logfile *lf, char *buf, size_t len)
{
    if (lf->ctx)
	lf->ctx->buf = buffer_write(lf->ctx->buf, buf, len);
}

static void log_write_syslog(struct logfile *lf, char *buf, size_t len)
{
    if (lf->ctx)
	lf->ctx->buf = buffer_write(lf->ctx->buf, buf, len);
}

void log_write(rb_tree_t * rbt, char *buf, size_t len)
{
    rb_node_t *rbn, *rbnext;
    char ebuf[8192];
    char *e = ebuf, *b = buf;
    size_t i, elen;

    for (i = 0, elen = 0; i < len && elen < sizeof(ebuf) - 4; i++, b++)
	if (*b == '\\') {
	    *e++ = *b;
	    *e++ = *b;
	    elen += 2;
	} else if (isprint((int) *b)) {
	    *e++ = *b;
	    elen++;
	} else {
	    *e++ = '\\';
	    *e++ = '0' + (7 & (*b >> 6));
	    *e++ = '0' + (7 & (*b >> 3));
	    *e++ = '0' + (7 & *b);
	    elen += 4;
	}

    for (rbn = RB_first(rbt); rbn; rbn = rbnext) {
	struct logfile *lf = RB_payload(rbn, struct logfile *);
	rbnext = RB_next(rbn);
	lf->log_write(lf, ebuf, elen);
    }
}

void log_flush(rb_tree_t * rbt)
{
    rb_node_t *rbn, *rbnext;
    for (rbn = RB_first(rbt); rbn; rbn = rbnext) {
	struct logfile *lf = RB_payload(rbn, struct logfile *);
	rbnext = RB_next(rbn);
	lf->log_flush(lf);
    }
}

static void log_flush_async(struct logfile *lf __attribute__ ((unused)))
{
    lf->log_write(lf, "\n", 1);
}

static void log_flush_syslog(struct logfile *lf __attribute__ ((unused)))
{
    if (lf->ctx && lf->ctx->buf) {
	int len = (int) buffer_getlen(lf->ctx->buf);
	syslog(lf->syslog_priority, "%.*s", len, lf->ctx->buf->buf);
	lf->ctx->buf = buffer_release(lf->ctx->buf, (off_t *) & len);
    }
}

static void log_flush_sync(struct logfile *lf)
{
    lf->log_write(lf, "\n", 1);
    logwrite_sync(lf->ctx, lf->ctx->fd);
}

int logs_flushed(void)
{
    rb_node_t *rbn, *rbnext;
    for (rbn = RB_first(config.logfiles); rbn; rbn = rbnext) {
	struct logfile *lf = RB_payload(rbn, struct logfile *);
	rbnext = RB_next(rbn);

	if (!lf->flag_pipe && !lf->flag_sync && lf->ctx && buffer_getlen(lf->ctx->buf))
	    return 0;
    }
    return -1;
}

int compare_log(const void *a, const void *b)
{
    return strcmp(((struct logfile *) a)->name, ((struct logfile *) b)->name);
}

void log_add(rb_tree_t ** rbt, char *s, tac_realm * r)
{
    size_t i = 0;
    rb_node_t *rbn;
    struct logfile *lf = alloca(sizeof(struct logfile));

    if (!*rbt)
	*rbt = RB_tree_new(compare_log, NULL);

    lf->name = s;

    if ((rbn = RB_search(config.logfiles, lf)))
	lf = RB_payload(rbn, struct logfile *);
    else {
	lf = calloc(1, sizeof(struct logfile));
	if (!strcmp(s, codestring[S_syslog])) {
	    lf->flag_syslog = BISTATE_YES;
	    lf->log_write = &log_write_syslog;
	    lf->log_flush = &log_flush_syslog;
	    lf->syslog_priority = common_data.syslog_level | common_data.syslog_facility;
	} else {
	    lf->flag_staticpath = (strchr(s, '%') == NULL);
	    lf->flag_pipe = 0;
	    lf->flag_sync = 0;
	    switch (*s) {
	    case '>':
		i++;
		lf->log_write = &log_write_sync;
		lf->log_flush = &log_flush_sync;
		lf->flag_sync = BISTATE_YES;
		break;
	    case '|':
		i++;
		lf->flag_pipe = BISTATE_YES;
	    default:
		lf->log_write = &log_write_async;
		lf->log_flush = &log_flush_async;
		break;
	    }
	}
	lf->name = strdup(s);
	lf->format = lf->name + i;
	lf->date_format = r->date_format;
	lf->log_separator = r->log_separator;
	lf->log_separator_len = r->log_separator_len;
	RB_insert(config.logfiles, lf);
    }
    RB_insert(*rbt, lf);
}
