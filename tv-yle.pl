#!/usr/bin/perl
# -*- mode: cperl; cperl-indent-level: 4 -*-
# $ tv-yle.pl $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
# Created: Fri 04 Dec 2015 18:53:41 EET too
# Last modified: Sun 31 Jan 2016 21:04:45 +0200 too

# Tämän lisenssi: PD
# * https://creativecommons.org/publicdomain/zero/1.0/deed.fi
# * https://creativecommons.org/publicdomain/zero/1.0/legalcode.fi

use 5.8.1;
use strict;
use warnings;

my %kanavat = qw[
	1  http://yletv-lh.akamaihd.net/i/yletv1hls_1@103188/master.m3u8
	2  http://yletv-lh.akamaihd.net/i/yletv2hls_1@103189/master.m3u8
	t  http://yletv-lh.akamaihd.net/i/yleteemahls_1@103187/master.m3u8
	f  http://yletv-lh.akamaihd.net/i/ylefemfihls_1@103185/master.m3u8
  ];

die "Usage: $0 kanava\n" unless @ARGV;
my $kan = shift;
my $url = $kanavat{$kan};

die "'$kan': emme tunne" unless defined $url;

# mpv valitsi listan ensimmäisen, ja vlc "suurimman". mun htpc-ympäristölle
# sopi parhaiten tämä "576" -formaatti...
# .. ja nyt vlc valitsi "pienimmän", --preferred-resolution ei vaikuttanut

open P, '-|', qw/wget -O -/, $url or die;
while (<P>) {
    last if/RESOLUTION=1024x576/;
}
while (<P>) {
    next unless /^\s*(http:\S+)/;
    $url = $1;
    last;
}
close P;

exec qw/vlc -f --video-on-top --play-and-exit --global-key-quit q
	--network-caching 5000 --no-video-title-show/, $url;
die 'not reached';

exec qw/mpv -fs -x11-netwm=no -monitoraspect 16:9/, $url;
die 'not reached';

__END__
#EXTM3U
#EXTINF:0,Yle 1
http://yletv-lh.akamaihd.net/i/yletv1hls_1@103188/master.m3u8
#EXTINF:0,Yle 2
http://yletv-lh.akamaihd.net/i/yletv2hls_1@103189/master.m3u8
#EXTINF:0,Yle Teema
http://yletv-lh.akamaihd.net/i/yleteemahls_1@103187/master.m3u8
#EXTINF:0,Yle Fem (suomi)
http://yletv-lh.akamaihd.net/i/ylefemfihls_1@103185/master.m3u8
#EXTINF:0,Yle Fem (ruotsi)
http://yletv-lh.akamaihd.net/i/ylefemsehls_1@103186/master.m3u8
