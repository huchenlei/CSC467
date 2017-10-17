#!/usr/bin/perl
use strict;
use warnings;

my $project_root = $ENV{'HOME'} . '/csc467';
my $file_dir = "$project_root/test/inputs";
opendir my $dir, $file_dir or die "Cannot open dir: $! \n";
my @test_files = readdir $dir;
closedir $dir;

splice(@test_files, 0, 2); # remove first two terms which are ['.', '..']

chdir $project_root;
foreach my $test_file (@test_files) {
    my $result = `./compiler467 -Tp $file_dir/$test_file 2>&1`; # redirect STDERR to STDOUT
    my $assertion = $test_file =~ /^pass_/;
    if ($result =~ /(PARSER ERROR.+)/) {
        if ($assertion) {
            print $1 . "\nTest on $test_file failed\n";
        } else {
            print "Test on $test_file passed\n";
        }
    } else {
        if ($assertion) {
            print "Test on $test_file passed\n";
        } else {
            print "Test on $test_file failed. Expect PARSING ERROR\n";
        }
    }
}
