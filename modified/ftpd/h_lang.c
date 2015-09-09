/*
 * h_lang.c
 * (C)1999-2011 by Marc Huber <Marc.Huber@web.de>
 *
 * $Id: h_lang.c,v 1.10 2015/03/14 06:11:25 marc Exp marc $
 *
 */

#include "headers.h"

static const char rcsid[] __attribute__ ((used)) = "$Id: h_lang.c,v 1.10 2015/03/14 06:11:25 marc Exp marc $";

void h_lang(struct context *ctx, char *arg)
{
    char *t, **l;
    int i;

    DebugIn(DEBUG_PROC);

    if (!arg)
	arg = "EN";
    else if ((t = strchr(arg, '-')))
	*t = 0;

    for (l = lang, i = 0; *l && strcasecmp(arg, *l); l++, i++);

    if (*l) {
	ctx->lang = i;
	replyf(ctx, MSG_200_Language_set, *l);
    } else
	replyf(ctx, MSG_501_Language_not_supported, arg);

    DebugOut(DEBUG_PROC);
}
