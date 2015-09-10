/*
 * radmavis authserver=localhost:1812:mysecret ...
 *
 * $Id: radmavis.c,v 1.11 2015/03/14 06:11:28 marc Exp marc $
 */

#ifdef DEBUG
# define MAVISDEBUG 1
# undef DEBUG
#endif
#include <freeradius-client.h>
#undef DEBUG
#ifdef MAVISDEBUG
# undef MAVISDEBUG
# define DEBUG
#endif
#include <string.h>
#include "mavis.h"

static void set_rc(rc_handle * rh, char *a, char *v)
{
    if (!rc_add_config(rh, a, v, "config", 0))
	return;
    fprintf(stderr, "Unable to set '%s'\n", a);
    rc_destroy(rh);
    exit(-1);
}

int main(int argc __attribute__ ((unused)), char **argv)
{
    int result;
    rc_handle *rh;
    char buf[4096], *user = NULL, *pass = NULL;
    VALUE_PAIR *send = NULL, *received = NULL;

    argv++;
    rh = rc_new();
    rh = rc_config_init(rh);

    set_rc(rh, "auth_order", "radius");
    set_rc(rh, "login_tries", "4");
    set_rc(rh, "dictionary", "/usr/local/etc/radiusclient/dictionary");
    set_rc(rh, "seqfile", "/var/run/radius.seq");
    set_rc(rh, "radius_retries", "3");
    set_rc(rh, "radius_timeout", "5");
    set_rc(rh, "radius_deadtime", "10");

    while (*argv) {
	char *eq = strchr(*argv, '=');
	if (eq) {
	    *eq = 0;
	    set_rc(rh, *argv, eq + 1);
	    *eq = '=';
	} else {
	    fprintf(stderr, "Unable to parse '%s'\n", *argv);
	    rc_destroy(rh);
	    exit(-1);
	}
	argv++;
    }

    if (rc_read_dictionary(rh, rc_conf_str(rh, "dictionary")) != 0) {
	fprintf(stderr, "reading %s failed\n", rc_conf_str(rh, "dictionary"));
	return ERROR_RC;
    }


    while (fgets(buf, sizeof(buf), stdin)) {
	int mavis_attr;
	char mavis_val[4096];
	if (2 == sscanf(buf, "%d %s\n", &mavis_attr, mavis_val)) {
	    printf("%s", buf);
	    switch (mavis_attr) {
	    case AV_A_USER:
		user = strdup(mavis_val);
		break;
	    case AV_A_PASSWORD:
		pass = strdup(mavis_val);
		break;
	    default:;
	    }
	} else if (!strcmp(buf, "=\n")) {
	    uint32_t service;
	    if (user && pass) {
		rc_avpair_add(rh, &send, PW_USER_NAME, user, -1, 0);
		rc_avpair_add(rh, &send, PW_USER_PASSWORD, pass, -1, 0);
		service = PW_AUTHENTICATE_ONLY;
		rc_avpair_add(rh, &send, PW_SERVICE_TYPE, &service, -1, 0);
		*buf = 0;
		result = rc_auth(rh, 0, send, &received, buf);
		if (*buf)
		    printf("%d %s\n", AV_A_COMMENT, buf);
	    } else
		result = REJECT_RC;

	    switch (result) {
	    case OK_RC:
		printf("%d %s\n=%d\n", AV_A_DBPASSWORD, pass, MAVIS_FINAL);
		break;
	    case REJECT_RC:
		printf("=%d\n", MAVIS_FINAL);
		break;
	    case TIMEOUT_RC:
	    case BADRESP_RC:
	    case ERROR_RC:
		printf("=%d\n", MAVIS_TIMEOUT);
		break;
	    default:
		printf("=%d\n", MAVIS_DOWN);
		break;
	    }

	    fflush(stdout);

	    rc_avpair_free(send);
	    rc_avpair_free(received);
	    send = NULL, received = NULL;
	    if (user) {
		free(user);
		user = NULL;
	    }
	    if (pass) {
		free(pass);
		pass = NULL;
	    }
	} else {
	    fprintf(stderr, "%s: Protocol violation. Exiting.\n", argv[0]);
	    exit(-1);
	}
    }

    exit(-1);
}