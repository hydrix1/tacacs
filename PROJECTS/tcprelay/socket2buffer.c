/*
 * socket2buffer.c
 * (C)1999-2011 by Marc Huber <Marc.Huber@web.de>
 * All rights reserved.
 *
 * $Id: socket2buffer.c,v 1.11 2015/03/14 06:11:31 marc Exp marc $
 *
 */

#include "headers.h"
#include "misc/io.h"

static const char rcsid[] __attribute__ ((used)) = "$Id: socket2buffer.c,v 1.11 2015/03/14 06:11:31 marc Exp marc $";

void socket2buffer(struct context *ctx, int cur)
{
    int fd_out;
    ssize_t l;
    struct buffer *b;

    io_sched_renew_proc(ctx->io, ctx, (void *) cleanup);

    DebugIn(DEBUG_NET);

    fd_out = (cur == ctx->ifn) ? ctx->ofn : ctx->ifn;
    io_clr_i(ctx->io, cur);
    b = buffer_get();

#ifdef WITH_SSL
    if (ctx->ssl)
	l = io_SSL_read(ctx->ssl, b->buf, b->size, ctx->io, cur, (void *) socket2buffer);
    else
#endif
	l = read(cur, b->buf, b->size);

    if (l > 0) {
	b->length = l;
	if (cur == ctx->ifn)	/* read from ifn, write to bufo */
	    ctx->bufo = buffer_append(ctx->bufo, b);
	else			/* read from ofn, write to bufi */
	    ctx->bufi = buffer_append(ctx->bufi, b);
	io_set_o(ctx->io, fd_out);
    } else
	buffer_free(b);


    if (l <= 0 && errno != EAGAIN && errno != EINTR)
	cleanup(ctx, cur);

    DebugOut(DEBUG_NET);
}
