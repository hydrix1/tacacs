#!/usr/bin/perl -w
# install_ascii.pl
# (C)2011 by Marc Huber <Marc.Huber@web.de>
# All rights reserved.
#
# $Id: install_ascii.pl,v 1.11 2013/07/28 06:17:53 marc Exp marc $

use strict;
use Digest::MD5 qw(md5_hex);
use Getopt::Std;

my %opts;

getopts('bm:o:g:R:S:', \%opts);

my $do_backup = exists($opts{'b'}) ? 1 : undef;
my $mode = exists($opts{'m'}) ? oct($opts{'m'}) : undef;
my $owner = exists($opts{'o'}) ? $opts{'o'} : undef;
my $group = exists($opts{'g'}) ? $opts{'g'} : undef;
my $REMOVELINE = exists($opts{'R'}) ? $opts{'R'} : undef;
my @SUBSTITUTE = exists($opts{'S'}) ? split(/\n/, $opts{'S'}) : ();

my $uid = $>;
my $gid = $(;

if (defined($owner)) {
	if ($owner =~ /^\d+$/) {
		$uid = $owner;
	} else {
		(undef,undef,$uid,$gid) = getpwnam($owner)
			or die "$owner not in passwd file";
	}
}

if (defined($group)) {
	if ($group =~ /^\d+$/) {
		$gid = $group;
	} else {
		(undef,undef,$gid) = getgrnam($group)
			or die "$group not in group file";
	}
}

die "Bad usage" if $#ARGV < 1;

my $target = pop @ARGV;
my $target_dir = undef;
$target_dir = $target if (-d $target || $target =~ /\/$/ || $#ARGV > 1);

if (defined $target_dir) {
	my @D = split(/\//, $target_dir);
	my $d = "";
	while(@D) {
		$d .= "/" . shift @D;
		mkdir($d, 0755);
	}
}

$/ = "\n# MD5SUM: ";

foreach my $f (@ARGV) {
	my ($F, $body, $digest, $digest_old);
	$target = "$target_dir/$f" if defined($target_dir);
        $mode = 0777 & (stat($f))[2] unless defined $mode;
	my $sum_should = "";
	my $sum_real = "";
	if (-f $target) {
		open $F, "<$target" or die "open $target failed";
		$body = <$F>;
		$body =~ s/$\/$//;
		$digest_old = <$F>;
		close $F;
		chop $digest_old;
		if ($digest_old ne md5_hex($body)) {
			printf STDERR "skipping $target (modified or no checksum)\n";
			next;
		}
	}
	open $F, "<$f" or die "open $f failed";
	$body = <$F>;
	close $F;
	if (defined($REMOVELINE)) {
		$body = join("\n", grep {!/$REMOVELINE/} split /\n/, $body);
	}
	foreach my $s (@SUBSTITUTE) {
		$s =~ /^s(.)/ or next;
		my ($d) = $1;
		$s =~ /s\Q$d\E([^\Q$d\E]*)\Q$d\E([^\Q$d\E]*)\Q$d\E([^\Q$d\E]*)$/;
		my ($a1, $a2) = ($1, $2);
		$body =~ s/\Q$a1\E/$a2/m;
	}

	$digest = md5_hex($body);

	$body .= "$/$digest\n";

	open $F, ">$target.$digest" or die "open $target.$digest failed";
	chmod $mode, $F;
	chown $uid, $gid, $F;
	print $F $body;
	close $F;

	rename($target, "$target.$digest_old") if defined($do_backup) && defined($digest_old) && $digest ne $digest_old;
	rename("$target.$digest", $target);
}
exit(0);

