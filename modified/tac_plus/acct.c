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

/* 
   Copyright (c) 1995-1998 by Cisco systems, Inc.

   Permission to use, copy, modify, and distribute this software for
   any purpose and without fee is hereby granted, provided that this
   copyright and permission notice appear on all copies of the
   software and supporting documentation, the name of Cisco Systems,
   Inc. not be used in advertising or publicity pertaining to
   distribution of the program without specific prior permission, and
   notice be given in supporting documentation that modification,
   copying and distribution is by permission of Cisco Systems, Inc.

   Cisco Systems, Inc. makes no representations about the suitability
   of this software for any purpose.  THIS SOFTWARE IS PROVIDED ``AS
   IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
   WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "headers.h"
#include <sys/stat.h>
#include <fcntl.h>

static const char rcsid[] __attribute__ ((used)) = "$Id: acct.c,v 1.91 2015/03/14 06:10:27 marc Exp $";

int accounting_pak_looks_bogus(tac_pak_hdr * hdr)
{
    struct acct *acct = tac_payload(hdr, struct acct *);
    u_char *p = (u_char *) acct + TAC_ACCT_REQ_FIXED_FIELDS_SIZE;
    int i;
    u_int len;
    u_int datalength = ntohl(hdr->datalength);

    /* Do some sanity checking on the packet */
    len = TAC_ACCT_REQ_FIXED_FIELDS_SIZE + acct->user_len + acct->port_len + acct->rem_addr_len + acct->arg_cnt;

    for (i = 0; i < (int) acct->arg_cnt && len < datalength; i++)
	len += p[i];

    return (i != (int) acct->arg_cnt || len != datalength);
}

void accounting(tac_session * session, tac_pak_hdr * hdr)
{
    rb_tree_t *rbt = session->ctx->aaa_realm->acct;
    report(session, LOG_DEBUG, DEBUG_ACCT_FLAG, "Start accounting request");

    if (rbt) {
	struct acct *acct = tac_payload(hdr, struct acct *);
	int i;
	char *acct_type, *username, *portname, *argstart;
	u_char *argsizep;
	u_char *p = (u_char *) acct + TAC_ACCT_REQ_FIXED_FIELDS_SIZE + acct->arg_cnt;

	if (acct->flags & TAC_PLUS_ACCT_FLAG_STOP)
	    acct_type = "stop";
	else if (acct->flags & TAC_PLUS_ACCT_FLAG_START)
	    acct_type = "start";
	else if (acct->flags & TAC_PLUS_ACCT_FLAG_WATCHDOG)
	    acct_type = "update";
	else
	    acct_type = "unknown";

	log_start(rbt);
	log_write_date(rbt);

	log_write(rbt, session->ctx->nas_address_ascii, strlen(session->ctx->nas_address_ascii));

	log_write_separator(rbt);
	log_write(rbt, (char *) p, acct->user_len);
	username = (char *) p;
	p += acct->user_len;

	log_write_separator(rbt);
	log_write(rbt, (char *) p, acct->port_len);
	portname = (char *) p;
	p += acct->port_len;

	log_write_separator(rbt);
	log_write(rbt, (char *) p, acct->rem_addr_len);
	p += acct->rem_addr_len;

	log_write_separator(rbt);
	log_write(rbt, acct_type, strlen(acct_type));

	argsizep = (u_char *) acct + TAC_ACCT_REQ_FIXED_FIELDS_SIZE;
	argstart = (char *) p;

	for (i = 0; i < (int) acct->arg_cnt; i++) {
	    log_write_separator(rbt);
	    log_write(rbt, (char *) p, *argsizep);
	    p += *argsizep++;
	}

	log_flush(rbt);

	if (acct->flags & TAC_PLUS_ACCT_FLAG_STOP) {
	    p = (u_char *) argstart;
	    argsizep = (u_char *) acct + TAC_ACCT_REQ_FIXED_FIELDS_SIZE;
	    for (i = 0; i < (int) acct->arg_cnt; i++) {
		if (!strcmp((char *) p, "service=exec")) {
		    char *mu = alloca(acct->user_len + 1);
		    char *mp = alloca(acct->port_len + 1);
		    strncpy(mu, username, acct->user_len);
		    mu[acct->user_len] = 0;
		    strncpy(mp, portname, acct->port_len);
		    mp[acct->port_len] = 0;
		    tac_script_set_exec_context(session, mu, mp, NULL);
		    break;
		}
		p += *argsizep++;
	    }
	}
    }

    send_acct_reply(session, TAC_PLUS_ACCT_STATUS_SUCCESS, NULL, NULL);
}
