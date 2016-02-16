#!/usr/bin/perl
# -*- mode: cperl; cperl-indent-level: 4 -*-
# $ testaa-regex.pl $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2016 Tomi Ollila
#	    All rights reserved
#
# Created: Tue 12 Jan 2016 22:30:52 EET too
# Last modified: Sat 06 Feb 2016 11:56:29 +0200 too

use 5.8.1;
use strict;
use warnings;

unlink './testaa-regex';
END { unlink './testaa-regex'; }
open P, '|-', qw/gcc -x c -Wall -Wextra -o testaa-regex -/ or die;
select P;

open I, '<',  'mm-selain.c' or die;

print "#include <$_.h>\n" foreach (qw/stdio stdlib regex/);
print "\nstruct { regex_t preg; } G;\n\n";
# good enough for this (i.e. do {} while (0) not needed.
print "#define die(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__); exit(1)\n\n";
while (<I>) {
    print($_), print(STDOUT $_), last if /static void doreg/;
}
while (<I>) { print $_; print STDOUT $_; last if /^}/; }
close I;

open I, '<', $0 or die;
while (<I>) { last if /__E[N]D__/; }
while (<I>) { print $_; }
close I;
close P;
die if $?;

if (fork) {
    exec './testaa-regex';
    die 'not reached';
}
select undef, undef, undef, 0.01;

__END__

int main(void)
{
    doreg();

    char buf[1024];
    while (fgets(buf, sizeof buf, stdin) != NULL) {
        char *p = buf;
        while (*p) {
            if (*p < 32)
                *p = '\0';
            else p++;
        }
        printf("regexec() returned %d\n", regexec(&G.preg, buf, 0, NULL, 0));
    }
    return 0;
}
