#!/usr/bin/perl
use strict;
use warnings;

if ($#ARGV < 0) {
    die "Usage: perl test.pl <test number>\n";
}

my $test_num = $ARGV[0];
my $demo_path = "./Demos/Demo$test_num";
`make`;
`./compiler467 -Dx $demo_path/shader.frag -O $demo_path/frag.txt`;

chdir $demo_path;
`make`;
print `./shader frag.txt`;
