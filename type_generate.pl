use strict;
use warnings;

my @types = qw [
T_VOID
T_INT
T_BOOL
T_FLOAT
T_VEC2
T_VEC3
T_VEC4
T_BVEC2
T_BVEC3
T_BVEC4
T_IVEC2
T_IVEC3
T_IVEC4
];

foreach my $type (@types) {
    $type =~ /T_([\w\d]+)/;
    print lc($1) . "/{DELIM} {yTRACE($type); return $type;}\n";
}
