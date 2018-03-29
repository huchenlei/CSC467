#!/usr/bin/perl
use strict;
use warnings;

if ($#ARGV < 0) {
    die "Usage: perl test.pl <test number>\n";
}

my $test_num = $ARGV[0];
my $demo_path = "./Demos/Demo$test_num";

my %binary_name = (
    "1" => "shader",
    "2" => "phong",
);

`make`;
print `./compiler467 -Dx $demo_path/$binary_name{$test_num}.frag -O $demo_path/frag.txt`;

chdir $demo_path;
`make`;
print `./$binary_name{$test_num} frag.txt`;
