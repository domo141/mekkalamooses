#!/usr/bin/perl
# -*- mode: cperl; cperl-indent-level: 4 -*-
# $ mm-kattele.pl $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2016 Tomi Ollila
#	    All rights reserved
#
# Created: Mon 29 Feb 2016 19:50:38 EET too
# Last modified: Sat 19 Mar 2016 17:25:59 +0200 too

use 5.8.1;
use strict;
use warnings;

my $hak = $ENV{MM_TIEDOSTOHAKEMISTO};
die "tiedostohakemisto ei tiedossa\n" unless defined $hak and $hak;

sub gdie(@)
{
    close STDOUT;
    exec './mm-viesti', 'huomaa', @_;
    die "exec failed: $!\n";
}

gdie "'$hak'", 'puuttuu (ei latauksia?)' unless -d $hak;
gdie 'hakemistoon', "'$hak'", 'pääsy estetty', $! unless chdir $hak;

unless (@ARGV)
{
    my $pfx = ' / ';
    my $pfl = 3;
    my @wnbs;

    while (<*>) {
	if (/[.]srt$/) {
	    if (substr($_, 0, $pfl) eq $pfx) {
		$_ = substr $_, $pfl;
		s/[.]?srt$//;
		print "/$_";
	    }
	    else {
		push @wnbs, $_;
	    }
	    next;
	}
	if (/(.*)[.](?:flv|mp4)/) {
	    $pfl = length() - 3;
	    $pfx = substr $_, 0, $pfl;
	    print "/\n//", $_;
	    foreach (@wnbs) {
		if (substr($_, 0, $pfl) eq $pfx) {
		    $_ = substr $_, $pfl;
		    s/[.]?srt$//;
		    print "/$_";
		}
	    }
	    @wnbs = ();
	}
    }
    print "/\n";

    exit
}

die "käyttö: komento tiedostonnimi [tp]" unless @ARGV >= 2;

if ($ARGV[0] eq 'poista')
{
    $_ = $ARGV[1];
    s/[.][^.]+$// or die "Outo tiedostonnimi: $ARGV[1]\n";
    my $fn = $_;
    (length) >= 4 or die "Yllättävän lyhyt tiedostonnimi: $ARGV[1]\n";
    my @lt = localtime;
    my @wd = qw/su ma ti ke to pe la/;
    my $day = sprintf
      "%d-%02d-%02d-%s", $lt[5] + 1900, $lt[4] + 1, $lt[3], @wd[$lt[6]];
    my $dir = "kesken/p-$day";
    unless (-d $dir) {
	mkdir $dir or die "Hakemiston '$dir' luonti ei onnistunut: $!\n";
    }
    #print "$fn, ", <$fn*>, "\n";
    rename $_, "$dir/$_" for (<$fn*>);
    exit
}
sub getsrt()
{
    $_ = $ARGV[1];
    s/[.][^.]+$// or die "Outo tiedostonnimi: $ARGV[1]\n";
    return $_ . '.srt' if $ARGV[2] eq '';
    return $_ . '.' . $ARGV[2] . '.srt';
}
if ($ARGV[0] eq 'vlclla')
{
    my @opts;
    push @opts, ('--sub-file', getsrt) if defined $ARGV[2];

    exec 'vlc', qw/-f --play-and-exit --no-video-title-show --global-key-quit q
		   --video-on-top --sub-autodetect-fuzzy 0/, @opts, $ARGV[1];
    die 'ei sa[a]vuteta';
}
if ($ARGV[0] eq 'mplayerillä')
{
    my @opts;
    # unfortunately mplayer & mplayer2 do not share same command line option
    # here (mplayer2 does not need it) -- and then there are -subcp vs --subcp
    push @opts, '-utf8' unless -d '/usr/share/mplayer2';  # XXX dummy heuristic
    push @opts, ('-sub', getsrt) if defined $ARGV[2];

    exec 'mplayer', qw/-fs -quiet -osd-duration 2000 -af scaletempo -ontop
		       -noautosub -cache 8192 -cache-min 1/, @opts, $ARGV[1];
    die 'ei sa[a]vuteta';
}

die "'$ARGV[0]': komentoa ei tunneta";
