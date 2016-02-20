#!/usr/bin/perl
# -*- mode: cperl; cperl-indent-level: 4 -*-
# $ mm-lataaja.pl $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2016 Tomi Ollila
#	    All rights reserved
#
# Created: Wed 13 Jan 2016 20:18:53 EET too
# Last modified: Thu 18 Feb 2016 21:20:16 +0200 too

# Licensed under GPLv3

use 5.8.1;
use strict;
use warnings;
use Cwd;

#my $bitrate = 'best';
my $bitrate = '2000';  # yritän rajoittaa max 1280x720 mun hitaalle htpc:lle

sub md($)
{
    unless (-d $_[0]) {
	mkdir $_[0], 0755 or die "mkdir ei onnistu '$_[0]':lle: $!\n";
    }
}

sub xexec(@)
{
    print "\nSuoritetaan @_\n";
    exec @_;
    die 'not reached';
}

my $hak = $ENV{MM_TIEDOSTOHAKEMISTO};
die "tiedostohakemisto ei tiedossa\n" unless defined $hak and $hak;

my $cwd = cwd();

my $yle_dl = 'yle-dl';
my @adobe_hsd;
if (-f "./yle-dl/$yle_dl") {
    my $ahsd = 'yle-dl/AdobeHDS.php';
    die "'$ahsd' ei sijaize $yle_dl:n kaverina polussa $cwd" unless -f $ahsd;
    $yle_dl = $cwd.'/yle-dl/'.$yle_dl;
    @adobe_hsd = ( '--adobehds', "php $cwd/$ahsd" )
}

md $hak;
md "$hak/kesken";

# when $url is updated, update doreg() in mm-selain.c too
$_ = $ARGV[0]; s/[?#].*//; my $url = $_; s|/+$||; s|.*/||;
my $d = $_;
my $td = "$hak/kesken/$d";

{
    open P, '-|', qw/pgrep mm-lataaja.pl/ or die;
    my $pc = 0;
    while (<P>) {
	$pc++ if /^\d+$/;
    }
    close P;
    if ($pc != 1) {
	exit if fork;
	if ($pc == 0) {
	    exec './mm-viesti', $url, 'lataajassa virhe,', 'ei löydä itseään';
	    die 'not reached';
	}
	exec './mm-viesti', $url, 'Laitettaisiin tämä jonoon, mutta',
	  'kun sitä ei oo vielä toteutettu';
	die 'not reached';
    }
}

#sleep 20; exit 0;

if (open I, '<', "$hak/kesken/jono") {
    #print "$hak/kesken/jono\n";
    while (<I>) {
	if (index($_, $d) >= 0) {
	    print "$d ", $_;
	    /(\d\d\d\d[-\d].*$d)/;
	    close I;
	    exec './mm-viesti', $url, "'$d' on jo ladattu:", '', $1, '',
	      'poista tieto jonosta jos virheellinen';
	    die 'not reached';
	}
    }
    close I;
}

my @lt = localtime;
my @wds = qw/su ma ti ke to pe la su/;
my $date = sprintf "%d-%02d-%02d (%s) %02d:%02d:%02d",
  $lt[5] + 1900, $lt[4] + 1, $lt[3], $wds[$lt[6]], $lt[2], $lt[1], $lt[0];

open O, '>>', "$hak/kesken/jono" || die $!;
print O ".  $date  $url\n";
close O;

md $td;
chdir $td or die "Cannot chdir to '$td': $!\n";

unless (fork) {
    system qw/pkill -USR1 mm-tausta/;
    print "\nHuomaa: jotkut lataukset ovat aivan hiljaisia\n";
    #exec qw/urxvt -hold -e sh -c/, "echo $yle_dl";
    xexec $yle_dl, qw/--latestepisode --backend adobehdsphp --hardsubs
		      --maxbitrate/, $bitrate, @adobe_hsd, $url;
    # not reached
    xexec qw/urxvt -T lataaja -g 80x24 -fg limegreen -bg black +sb -e/,
      $yle_dl, qw/--latestepisode --backend adobehdsphp/, @adobe_hsd, $url;
}
sleep 1 while wait == -1;
exit if $?;

# https://en.wikipedia.org/wiki/Zeller%27s_congruence
sub zeller_paiva($$$)
{
    my ($y, $m) = ($_[0], $_[1]);
    if ($m == 1 || $m == 2) {
	$m += 12;
	$y -= 1;
    }
    my $wd = int($_[2] + int(($m + 1) * 26 / 10) + $y + int ($y / 4)
		 + 6 * int ($y / 100) + int ($y / 400)) % 7;
    return qw/la su ma ti ke to pe/[$wd];
}

for (<*.*>) {
    next if /frag/i;
    next unless -f $_;
    my $s = $_;
    # toivottavasti aikaformaatti säilyy...
    if (s/-(\d\d\d\d)-(\d\d)-(\d\d)T(\d\d):(\d\d):[^.]+(.*)//) {
	my $zvpv = zeller_paiva $1, $2, $3;
	my $aika = $1.$2.$3.'-'.$zvpv.'-'.$4.$5;
	my $lopp = $6;
	$_ = $_ . '-' . $aika unless s/\s*:\s*/-$aika-/;
	$_ = $_ . $lopp;
    }
    tr/: /__/; s/__+/_/g;
    rename $s, "../../$_";
    print "Siirretty nimelle $_\n";
}
chdir '..';
rmdir $d;
chdir $cwd;
exit if fork;
exec './mm-viesti', $url, 'lataus suoritettu';
