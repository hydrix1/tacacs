/*
 * libmavis.c
 * (C)1998-2011 by Marc Huber <Marc.Huber@web.de>
 *
 * $Id: libmavis.c,v 1.26 2015/03/14 06:11:27 marc Exp marc $
 *
 */

#define __MAVIS_MAIN__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <sysexits.h>
#include "misc/io_sched.h"
#include "misc/memops.h"
#include "misc/mymd5.h"
#include "log.h"
#include "misc/base64.h"
#include "debug.h"
#include "mavis.h"
#include "misc/version.h"

static const char rcsid[] __attribute__ ((used)) = "$Id: libmavis.c,v 1.26 2015/03/14 06:11:27 marc Exp marc $";

extern struct module_defn mod_anonftp;
extern struct module_defn mod_asciiftp;
extern struct module_defn mod_auth;
extern struct module_defn mod_cache;
extern struct module_defn mod_external;
extern struct module_defn mod_groups;
extern struct module_defn mod_limit2;
extern struct module_defn mod_limit;
extern struct module_defn mod_log;
extern struct module_defn mod_null;
extern struct module_defn mod_pam;
extern struct module_defn mod_remote;
extern struct module_defn mod_system;
extern struct module_defn mod_tee;
extern struct module_defn mod_userdb;

static struct module_defn * all_modules[]  = 
{
    &mod_anonftp,
    &mod_asciiftp,
    &mod_auth,
    &mod_cache,
    &mod_external,
    &mod_groups,
    &mod_limit2,
    &mod_limit,
    &mod_log,
    &mod_null,
#ifdef HAVE_SECURITY_PAM_APPL_H
    &mod_pam,
#endif
    &mod_remote,
    &mod_system,
    &mod_tee,
    &mod_userdb
};

int mavis_method_add(mavis_ctx ** mcx, struct io_context *ioctx, char *path, char *id)
{
    int i;
    void *handle = NULL;
    void *(*mn) (void *, struct io_context *, char *) = NULL;

#if 0
    Debug((DEBUG_MAVIS, "+ %s(%s)\n", __func__, path));

    handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);

    if (!handle) {
	Debug((DEBUG_MAVIS, "- %s(%s): dlopen failed.\n", __func__, path));
	return -1;
    }

    if (!(mn = (void *(*)(void *, struct io_context *, char *))
	  dlsym(handle, DLSYM_PREFIX "Mavis_new"))) {
	Debug((DEBUG_MAVIS, "- %s(%s): dlsym (%s) failed: %s\n", __func__, path, DLSYM_PREFIX "Mavis_new", dlerror()));
	return -1;
    }
#endif
    for (i = 0; i < sizeof(all_modules) / sizeof(struct module_defn *); i++)
    {
        if (strcmp(all_modules[i]->name, path) == 0)
        {
            mn = all_modules[i]->new;
        }
    }

    if (mn == NULL)
    {
        return -1;
    }

    if (!*mcx)
	*mcx = mn(handle, ioctx, id);
    else
	(*mcx)->append(*mcx, mn(handle, ioctx, id));

    Debug((DEBUG_MAVIS, "- %s (OK)\n", __func__));
    return 0;
}

int mavis_init(mavis_ctx * mcx, char *version)
{
    int result = MAVIS_INIT_OK;

    DebugIn(DEBUG_MAVIS);

    mavis_check_version(version);

    if (!mcx) {
	Debug((DEBUG_MAVIS, "- %s: FATAL: no modules configured\n", __func__));
	logmsg("Fatal: No modules configured");
	exit(EX_USAGE);
    }

    result = mcx->init(mcx);
    Debug((DEBUG_MAVIS, "- %s = %d\n", __func__, result));
    return result;
}

int mavis_drop(mavis_ctx * mcx)
{
    void *handle = NULL;

    DebugIn(DEBUG_MAVIS);

    if (mcx)
	handle = mcx->drop(mcx);
#if 0
    if (handle)
	dlclose(handle);
#endif

    DebugOut(DEBUG_MAVIS);
    return 0;
}

int mavis_parse(mavis_ctx * mcx, struct sym *sym, char *id)
{
    int result = MAVIS_CONF_ERR;

    DebugIn(DEBUG_MAVIS);

    if (mcx)
	result = mcx->parse(mcx, sym, id);
    DebugOut(DEBUG_MAVIS);
    return result;
}

static int mavis_sanitycheck(mavis_ctx * mcx, av_ctx * ac)
{
    if (!mcx) {
	av_set(ac, AV_A_RESULT, AV_V_RESULT_ERROR);
	av_set(ac, AV_A_COMMENT, "no modules installed");
	return -1;
    }
    if (ac->arr[AV_A_TYPE] && ((!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_FTP) && ac->arr[AV_A_USER]
				&& ac->arr[AV_A_PASSWORD] && ac->arr[AV_A_IPADDR])
			       || (!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_WWW)
				   && ac->arr[AV_A_USER] && ac->arr[AV_A_PASSWORD])
			       || (!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_RADIUS)
				   && ac->arr[AV_A_USER] && ac->arr[AV_A_PASSWORD])
			       || (!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_TACPLUS)
				   && ac->arr[AV_A_USER] && ac->arr[AV_A_TACTYPE])
			       || (!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_LOGIN)
				   && ac->arr[AV_A_USER] && ac->arr[AV_A_PASSWORD]
				   && ac->arr[AV_A_IPADDR])
			       || (!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_POP3)
				   && ac->arr[AV_A_TIMESTAMP] && ac->arr[AV_A_USER]
				   && (ac->arr[AV_A_DIGEST] || ac->arr[AV_A_PASSWORD])
				   && ac->arr[AV_A_IPADDR])
			       || (!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_TRANSPORT)
				   && ac->arr[AV_A_USER])
			       || (!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_CANONICAL)
				   && ac->arr[AV_A_USER])
			       || (!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_VIRTUAL)
				   && ac->arr[AV_A_USER])
			       || (!strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_POP3PATH)
				   && ac->arr[AV_A_USER])
			       || !strcmp(ac->arr[AV_A_TYPE], AV_V_TYPE_LOGSTATS)))
	return 0;
    av_set(ac, AV_A_RESULT, AV_V_RESULT_ERROR);
    av_set(ac, AV_A_COMMENT, "invalid request");
    return -1;
}

char *av_addserial(av_ctx * ac)
{
    if (!ac->arr[AV_A_SERIAL]) {
	u_char u[16];
	char b[30];
	size_t i, len = (int) sizeof(b);
	myMD5_CTX m;
	MD5Init(&m);
	for (i = 0; i < AV_A_ARRAYSIZE; i++)
	    if (ac->arr[i])
		MD5Update(&m, (u_char *) ac->arr[i], strlen(ac->arr[i]));
	MD5Final(u, &m);
	base64enc((char *) u, (size_t) 16, b, &len);
	av_set(ac, AV_A_SERIAL, b);
    }
    return ac->arr[AV_A_SERIAL];
}

int mavis_send(mavis_ctx * mcx, av_ctx ** ac)
{
    int result = MAVIS_IGNORE;

    DebugIn(DEBUG_MAVIS);

    if (mcx) {
	if (!mavis_sanitycheck(mcx, *ac)) {
	    av_addserial(*ac);
	    if (!strcmp((*ac)->arr[AV_A_TYPE], AV_V_TYPE_LOGSTATS))
		av_set(*ac, AV_A_RESULT, AV_V_RESULT_OK);

	    result = mcx->send(mcx, ac);

	    if (result == MAVIS_FINAL && !(*ac)->arr[AV_A_RESULT])
		av_set(*ac, AV_A_RESULT, AV_V_RESULT_NOTFOUND);
	}
    }
    Debug((DEBUG_MAVIS, "- %s (%d)\n", __func__, result));
    return result;
}

int mavis_cancel(mavis_ctx * mcx, void *app_ctx)
{
    int result = MAVIS_IGNORE;

    DebugIn(DEBUG_MAVIS);

    result = mcx->cancel(mcx, app_ctx);

    Debug((DEBUG_MAVIS, "- %s (%d)\n", __func__, result));
    return result;
}

int mavis_recv(mavis_ctx * mcx, av_ctx ** ac, void *app_ctx)
{
    int result;
    DebugIn(DEBUG_MAVIS);
    result = mcx->recv(mcx, ac, app_ctx);
    DebugOut(DEBUG_MAVIS);
    return result;
}

void av_clear(av_ctx * ac)
{
    DebugIn(DEBUG_AV);
    if (ac) {
	int i;

	for (i = 0; i < AV_A_ARRAYSIZE; i++)
	    Xfree(&ac->arr[i]);
    }
    DebugOut(DEBUG_AV);
}

void av_move(av_ctx * ac_out, av_ctx * ac_in)
{
    int i;

    DebugIn(DEBUG_AV);
    av_clear(ac_out);

    for (i = 0; i < AV_A_ARRAYSIZE; i++) {
	ac_out->arr[i] = ac_in->arr[i];
	ac_in->arr[i] = NULL;
    }

    DebugOut(DEBUG_AV);
}

void av_copy(av_ctx * ac_out, av_ctx * ac_in)
{
    int i;

    DebugIn(DEBUG_AV);
    av_clear(ac_out);

    for (i = 0; i < AV_A_ARRAYSIZE; i++) {
	Xfree(&ac_out->arr[i]);
	if (ac_in->arr[i])
	    ac_out->arr[i] = strdup(ac_in->arr[i]);
    }

    DebugOut(DEBUG_AV);
}

void av_merge(av_ctx * ac_out, av_ctx * ac_in)
{
    int i;

    DebugIn(DEBUG_AV);

    for (i = 0; i < AV_A_ARRAYSIZE; i++)
	if (!ac_out->arr[i] && ac_in->arr[i])
	    ac_out->arr[i] = strdup(ac_in->arr[i]);

    DebugOut(DEBUG_AV);
}

void av_set(av_ctx * ac, int av_attribute, char *av_value)
{
    if (av_attribute < 0 || av_attribute >= AV_A_ARRAYSIZE) {
	Debug((DEBUG_AV, "%s(%d) out of bounds\n", __func__, av_attribute));
	return;
    }

    Debug((DEBUG_AV, " %s(%s) = %-20s\n", __func__, av_char[av_attribute], av_value ? av_value : "(NULL)"));

    Xfree(&ac->arr[av_attribute]);

    ac->arr[av_attribute] = av_value ? Xstrdup(av_value) : NULL;
}

void av_setf(av_ctx * ac, int av_attribute, char *format, ...)
{
    size_t len = 1024, nlen;
    va_list ap;
    char *tmpbuf = alloca(len);

    va_start(ap, format);
    nlen = vsnprintf(tmpbuf, len, format, ap);
    va_end(ap);
    if (len <= nlen) {
	tmpbuf = alloca(++nlen);
	va_start(ap, format);
	vsnprintf(tmpbuf, nlen, format, ap);
	va_end(ap);
    }
    va_end(ap);
    av_set(ac, av_attribute, tmpbuf);
}

char *av_get(av_ctx * ac, int av_attribute)
{
    if (av_attribute < 0 || av_attribute > AV_A_ARRAYSIZE) {
	Debug((DEBUG_AV, "%s(%d): out of bounds\n", __func__, av_attribute));
	return NULL;
    }
#ifdef DEBUG
    if (ac->arr[av_attribute])
	Debug((DEBUG_AV, " %s(%s) = %-20s\n", __func__, av_char[av_attribute], ac->arr[av_attribute] ? ac->arr[av_attribute] : "(NULL)"));
#endif

    return ac->arr[av_attribute];
}

void av_dump(av_ctx * ac)
{
    int i;

    fprintf(stderr, "attribute-value-pairs:\n");
    for (i = 0; i < AV_A_ARRAYSIZE; i++)
	if (ac->arr[i])
	    fprintf(stderr, "%-20s%s\n", av_char[i], ac->arr[i]);
    fprintf(stderr, "\n");
}

int av_attribute_to_i(char *s)
{
    int i;

    for (i = 0; i < AV_A_ARRAYSIZE; i++)
	if (!strcasecmp(av_char[i], s))
	    return i;
    return -1;
}

int av_array_to_char(av_ctx * ac, char *buffer, size_t buflen, fd_set * set)
{
    int i, j;
    char *u;
    char *t = buffer;

    buffer[0] = 0;

    for (i = 0; i < AV_A_ARRAYSIZE; i++)
	if ((!set || FD_ISSET(i, set)) && (u = av_get(ac, i))) {
	    j = snprintf(t, (size_t) (buffer + buflen - t), "%d %s\n", i, u);
	    if (j >= buffer + buflen - t)
		return -1;
	    t += j;
	}

    return (int) (t - buffer);
}

int av_char_to_array(av_ctx * ac, char *buffer, fd_set * set)
{
    char *av_start = buffer;
    char *av_end = av_start;
    int av_attribute;
    char *av_value;

    while ((av_end = strchr(av_end, '\n'))) {
	*av_end = 0;
	av_value = strchr(av_start, ' ');
	if (av_value) {
	    *av_value++ = 0;
	    if ((1 == sscanf(av_start, "%d", &av_attribute)) && (!set || FD_ISSET(av_attribute, set)))
		av_set(ac, av_attribute, av_value);
	    *(av_value - 1) = ' ';
	}
	*av_end++ = '\n';
	av_start = av_end;
    }

    return 0;
}

av_ctx *av_new(void *cb, void *ctx)
{
    av_ctx *a = Xcalloc((size_t) 1, sizeof(av_ctx));
    a->app_cb = cb;
    a->app_ctx = ctx;
    return a;
}

void av_setcb(av_ctx * a, void *cb, void *ctx)
{
    a->app_cb = cb;
    a->app_ctx = ctx;
}

void av_free(av_ctx * ac)
{
    if (ac) {
	int i;
	for (i = 0; i < AV_A_ARRAYSIZE; i++)
	    Xfree(&ac->arr[i]);
	free(ac);
    }
}

int mavis_check_version(char *version)
{
    if (strcmp(version, VERSION)) {
	logmsg("Warning: MAVIS library version mismatch (%s vs. %s). Expect trouble.", version, VERSION);
	return -1;
    }
    return 0;
}

void mavis_detach(void)
{
    int devnull;

    setsid();
    devnull = open("/dev/null", O_RDWR);
    dup2(devnull, 1);
    close(devnull);
    fcntl(0, F_SETFD, fcntl(0, F_GETFD, 0) | FD_CLOEXEC);
    fcntl(1, F_SETFD, fcntl(1, F_GETFD, 0) | FD_CLOEXEC);
}
