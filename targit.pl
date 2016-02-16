#!/usr/bin/perl
# -*- mode: cperl; cperl-indent-level: 4 -*-
# $ targit.pl $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2016 Tomi Ollila
#	    All rights reserved
#
# Created: Thu 28 Jan 2016 22:52:44 EET too
# Last modified: Sat 30 Jan 2016 14:55:27 +0200 too

use 5.8.1;
use strict;
use warnings;

use tarlisted;

open P, '-|', qw/git ls-tree -l master/ or die $!;
my @fida;
my %fida;
while (<P>) {
    /\d{3}(\d+)\s+blob\s+(\w+)\s+(\d+)\s+(\S+)/ or next;
    my $ref = [ $1, $2, $3, $4 ];
    push @fida, $ref;
    $fida{$4} = $ref;
}
close P;

# check a bit that we are in expected repository
foreach (qw/mm-selain.c mm-tausta.c tv-yle.pl mekkalamooses/) {
        die "File '$_' not found in repository.\n" unless defined $fida{$_};
}

open P, '-|', qw/git log --date-order --name-status
		 --format=%ct%x20%h%x20%ci master/
  or die $!;
my ($time, $ftime, $ahash, $day, $dayc);
sub settime() {
    $time = $1;
    ($ftime, $ahash, $day, $dayc) = ($1, $2, $3, 0) unless defined $day;
    $dayc++ if $day eq $3;
}
while (<P>) {
    settime, next if /^(\d+)\s+(\w+)\s+(\S+)/;
    if (/^\S\s+(\S+)$/) {
	my $ref = $fida{$1};
	next unless defined $ref;
	${$ref}[4] = $time;
	delete $fida{$1};
	#print scalar(%fida), " $1\n";
	last unless %fida;
	next;
    }
    next if /^$/;
    chomp;
    die "line $.: '$_' not empty\n";
}
my $version = "$day-$dayc-g$ahash";

#print "$version\n"; exit;

my @cf;
sub load_file($$$)
{
    @cf = ();
    my $s = 0;
    open P, '-|', qw/git cat-file -p/, $_[0];
    while (<P>) {
	push @cf, $_;
	$s += length($_);
    }
    close P;
    die "file '$_[2]' blob '$_[0]' size $s unexpected (!= $_[1])\n"
      unless $s == $_[1];
}

sub patch_file($)
{
    foreach (@cf) {
	if (/VS=.git/) {
	    my $ol = length $_;
	    $_ = " VS='$version' # version patched by $0\n";
	    my $l = length $_;
	    return $l - $ol;
	}
	#print $cf[$_]
    }
    die "$_[0] missing content";
}

my $dir = "mekkalamooses-$version";
tarlisted_open "$dir.wip", qw/xz -9/;

# directory
tarlisted_writehdr
tarlisted_mkhdr $dir, 0755, 0,0, 0, $ftime, 5, '','root','root';

foreach (@fida) {
    my ($perm, $blob, $size, $name, $mtime) = @{$_};
    load_file $blob, $size, $name;
    $size += patch_file $name if $name eq 'mm-tausta.c';
    $name = $dir.'/'.$name;
    tarlisted_writehdr
    tarlisted_mkhdr $name, oct($perm), 0,0, $size, $mtime, 0, '','root','root';
    tarlisted_stringfile join('', @cf), $size;
    print '.';
    #print " $_->[4]  $_->[0]  $_->[1]  $_->[2]  $_->[3]\n";
}
tarlisted_close;

rename "$dir.wip", "$dir.tar.xz";
print " wrote $dir.tar.xz\n";
