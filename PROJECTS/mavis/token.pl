#!/usr/bin/perl -w
# $Id: token.pl,v 1.83 2014/06/18 17:24:23 marc Exp marc $

use strict;

my @enum = ("S_unknown = 0");
my @array = ("\"<unknown>\"");

my ($k, $t);
my %K;

while (<DATA>){
	/^([^\s]+)\s+([^\s]+)/ or next;
	$K{$2} = $1;
}

my $i = 1;
for my $w (sort { $K{$a} cmp $K{$b} } keys %K){
	push @enum, $w;
	push @array, '"' . $K{$w} . '"';
	$i++;
}

open F, "> token.h" or die;
print F "extern char *codestring[];\n";
print F "enum token {\n\t" . join(",\n\t", @enum), ",\n\tS_null\n};\n";
close F;

open F, "> token.c" or die;
print F "#include <stdio.h>\nchar *codestring[] = {\n\t" . join(",\n\t", @array), ",\n\tNULL\n};\n";
close F;


__DATA__
7		S_7
<end-of-file>	S_eof
<pcre-regex>	S_slash
<string>	S_string
=		S_equal
ACCT		S_ACCT
ACL		S_ACL
ALL		S_ALL
AUTHEN		S_AUTHEN
AUTHOR		S_AUTHOR
AV		S_AV
BUFFER		S_BUFFER
CMD		S_CMD
CONFIG		S_CONFIG
CONTROL		S_CONTROL
FORK		S_FORK
HEX		S_HEX
INDEX		S_INDEX
LOCK		S_LOCK
MAVIS		S_MAVIS
NET		S_NET
NONE		S_NONE
PACKET		S_PACKET
PARSE		S_PARSE
PATH		S_PATH
PROC		S_PROC
REGEX		S_REGEX
LWRES		S_LWRES
accept		S_accept
reject		S_reject
access		S_access
accounting	S_accounting
acl		S_acl
address		S_address
address-mismatch	S_addressmismatch
alias		S_alias
all		S_all
allow-dotfiles	S_allowdotfiles
anon		S_anon
anonymous-enable	S_anonenable
augmented-enable	S_augmented_enable
any		S_any
arap		S_arap
arg		S_arg
ascii-size-limit	S_asciisizelimit
attribute	S_attr
aaa		S_aaa
auth		S_auth
authenticated	S_authenticated
authentication	S_authentication
authentication-failures S_authenticationfailures
authmode	S_authmode
authorization	S_authorization
auto-conversion	S_autoconversion
backend		S_backend
background	S_background
backlog		S_backlog
banner		S_banner
banner-action	S_banneraction
binary-only	S_binaryonly
blacklist	S_blacklist
blowfish	S_blowfish
buffer		S_buffer
bye		S_bye
ca-file		S_cafile
ca-path		S_capath
cache		S_cache
caseless	S_caseless
cert		S_cert
certfile	S_certfile
chalresp	S_chalresp
chap		S_chap
check		S_check
check-gid	S_checkgid
check-perm	S_checkperm
check-uid	S_checkuid
checksum	S_checksum
childs		S_childs
chmod-mask	S_chmodmask
chpass		S_chpass
chroot		S_chroot
ciphers		S_ciphers
clear		S_clear
client		S_client
client-only	S_clientonly
cmd		S_cmd
command		S_command
config		S_config
connection	S_connection
content		S_content
coredump	S_coredump
count		S_count
crypt		S_crypt
debug		S_debug
trace		S_trace
default		S_default
deflate		S_deflate
deflate-level	S_deflatelevel
delimiter	S_delimiter
deny		S_deny
depth		S_depth
des		S_des
directory	S_directory
dst		S_dst
enable		S_enable
event		S_event
exec		S_exec
expire		S_expire
expires		S_expires
facility	S_facility
fake-group	S_fakegroup
fake-owner	S_fakeowner
file		S_file
files-only	S_filesonly
follow		S_follow
from		S_from
ftpusers	S_ftpusers
goodbye		S_goodbye
greeting	S_greeting
group		S_group
groupid		S_groupid
gzip		S_gzip
hide-version	S_hideversion
home		S_home
host		S_host
hostname	S_hostname
id		S_id
ident		S_ident
idle		S_idle
in		S_in
include		S_include
inherit		S_inherit
instances	S_instances
key		S_key
keyfile		S_keyfile
level		S_level
limit		S_limit
listen		S_listen
local		S_local
log		S_log
logformat	S_logformat
login		S_login
logout		S_logout
maintainer	S_maintainer
mask		S_mask
mavis		S_mavis
max		S_max
max-attempts	S_maxattempts
member		S_member
message		S_message
mimetypes	S_mimetypes
min		S_min
mmap-size	S_mmapsize
mode		S_mode
module		S_module
ms-chap		S_mschap
nac		S_nac
nac-name	S_nacname
name		S_name
nas		S_nas
nas-name	S_nasname
nlst		S_nlst
no		S_no
noauthcache	S_noauthcache
noecho		S_noecho
none		S_none
not		S_not
old-draft	S_olddraft
opap		S_opap
optional	S_optional
orphan		S_orphan
out		S_out
pap		S_pap
passive		S_passive
passphrase	S_passphrase
passwd		S_passwd
password	S_password
path		S_path
period		S_period
permit		S_permit
pidfile		S_pidfile
pid-file	S_pid_file
port		S_port
proctitle	S_proctitle
profile		S_profile
prohibit	S_prohibit
prompt		S_prompt
protected	S_protected
protocol	S_protocol
purge		S_purge
readme		S_readme
readme-notify	S_readmenotify
readme-once	S_readmeonce
real		S_real
rebalance	S_rebalance
redirect	S_redirect
regex		S_regex
remote		S_remote
required	S_required
resolve-ids	S_resolveids
retire		S_retire
rewrite		S_rewrite
root		S_root
same		S_same
secure		S_secure
separation	S_separation
server		S_server
service		S_service
set		S_set
setenv		S_setenv
shape-bandwidth	S_shapebandwidth
shell		S_shell
shells		S_shells
site		S_site
size		S_size
spawn		S_spawn
src		S_src
ssl		S_ssl
sslusers	S_sslusers
stat		S_stat
state		S_state
subject		S_subject
substitute	S_substitute
sufficient	S_sufficient
symlinks	S_symlinks
syslog		S_syslog
tag		S_tag
template	S_template
clone		S_clone
time		S_time
timeout		S_timeout
timespec	S_timespec
tls		S_tls
transfer	S_transfer
transmission-mode	S_transmissionmode
transmit	S_transmit
tries		S_tries
type		S_type
type7-key	S_type7key
umask		S_umask
until		S_until
upload		S_upload
use-mmap	S_usemmap
use-sendfile	S_usesendfile
user		S_user
userid		S_userid
users		S_users
valid		S_valid
warn		S_warn
warning		S_warning
weight		S_weight
welcome		S_welcome
welcome-action	S_welcomeaction
working		S_working
yes		S_yes
z		S_z
zone		S_zone
{		S_openbra
}		S_closebra
!		S_exclmark
(		S_leftbra
)		S_rightbra
~		S_tilde
,		S_comma
&&		S_and
||		S_or
if		S_if
skip		S_skip
return		S_return
else		S_else
unset		S_unset
eval		S_eval
script		S_script
continue	S_continue
reset		S_reset
toupper		S_toupper
tolower		S_tolower
backoff		S_backoff
realm		S_realm
dns		S_dns
reverse-lookup	S_reverselookup
cleanup		S_cleanup
context		S_context
defined		S_defined
undef		S_undef
local		S_local
single-connection	S_singleconnection
may-close	S_mayclose
session		S_session
gcore		S_gcore
overload	S_overload
close		S_close
queue		S_queue
date		S_date
format		S_format
separator	S_separator
fallback	S_fallback
fallback-only	S_fallback_only
ipc		S_ipc
filter		S_filter
prefetch	S_prefetch
sticky		S_sticky
motd		S_motd
hushlogin	S_hushlogin
preload		S_preload
TCP		S_TCP
SCTP		S_SCTP
STREAM		S_STREAM
SEQPACKET	S_SEQPACKET
failed		S_failed
restriction	S_restriction
router		S_router
resolve		S_resolve
gid		S_gid
gids		S_gids
missing 	S_missing
conflicting 	S_conflicting
groups 		S_groups
retry 		S_retry
delay 		S_delay
bind		S_bind
debug-cmd	S_debug_cmd
single		S_single
process		S_process
aaa-realm	S_aaarealm
nac-realm	S_nacrealm
nas-realm	S_nasrealm
double-quote-values S_double_quote_values
mapping		S_mapping
tcp		S_tcp
keepalive	S_keepalive
interval	S_interval
regex-match-case S_regex_match_case
usage		S_usage

